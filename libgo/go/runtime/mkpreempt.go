// Copyright 2019 The Go Authors. All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

// +build ignore

// mkpreempt generates the asyncPreempt functions for each
// architecture.
package main

import (
	"flag"
	"fmt"
	"io"
	"log"
	"os"
	"strings"
)

// Copied from cmd/compile/internal/ssa/gen/*Ops.go

var regNames386 = []string{
	"AX",
	"CX",
	"DX",
	"BX",
	"SP",
	"BP",
	"SI",
	"DI",
	"X0",
	"X1",
	"X2",
	"X3",
	"X4",
	"X5",
	"X6",
	"X7",
}

var regNamesAMD64 = []string{
	"AX",
	"CX",
	"DX",
	"BX",
	"SP",
	"BP",
	"SI",
	"DI",
	"R8",
	"R9",
	"R10",
	"R11",
	"R12",
	"R13",
	"R14",
	"R15",
	"X0",
	"X1",
	"X2",
	"X3",
	"X4",
	"X5",
	"X6",
	"X7",
	"X8",
	"X9",
	"X10",
	"X11",
	"X12",
	"X13",
	"X14",
	"X15",
}

var out io.Writer

var arches = map[string]func(){
	"386":     gen386,
	"amd64":   genAMD64,
	"arm":     genARM,
	"arm64":   genARM64,
	"mips64x": func() { genMIPS(true) },
	"mipsx":   func() { genMIPS(false) },
	"ppc64x":  genPPC64,
	"riscv64": genRISCV64,
	"s390x":   genS390X,
	"wasm":    genWasm,
}
var beLe = map[string]bool{"mips64x": true, "mipsx": true, "ppc64x": true}

func main() {
	flag.Parse()
	if flag.NArg() > 0 {
		out = os.Stdout
		for _, arch := range flag.Args() {
			gen, ok := arches[arch]
			if !ok {
				log.Fatalf("unknown arch %s", arch)
			}
			header(arch)
			gen()
		}
		return
	}

	for arch, gen := range arches {
		f, err := os.Create(fmt.Sprintf("preempt_%s.s", arch))
		if err != nil {
			log.Fatal(err)
		}
		out = f
		header(arch)
		gen()
		if err := f.Close(); err != nil {
			log.Fatal(err)
		}
	}
}

func header(arch string) {
	fmt.Fprintf(out, "// Code generated by mkpreempt.go; DO NOT EDIT.\n\n")
	if beLe[arch] {
		base := arch[:len(arch)-1]
		fmt.Fprintf(out, "// +build %s %sle\n\n", base, base)
	}
	fmt.Fprintf(out, "#include \"go_asm.h\"\n")
	fmt.Fprintf(out, "#include \"textflag.h\"\n\n")
	fmt.Fprintf(out, "TEXT ·asyncPreempt(SB),NOSPLIT|NOFRAME,$0-0\n")
}

func p(f string, args ...interface{}) {
	fmted := fmt.Sprintf(f, args...)
	fmt.Fprintf(out, "\t%s\n", strings.Replace(fmted, "\n", "\n\t", -1))
}

func label(l string) {
	fmt.Fprintf(out, "%s\n", l)
}

type layout struct {
	stack int
	regs  []regPos
	sp    string // stack pointer register
}

type regPos struct {
	pos int

	op  string
	reg string

	// If this register requires special save and restore, these
	// give those operations with a %d placeholder for the stack
	// offset.
	save, restore string
}

func (l *layout) add(op, reg string, size int) {
	l.regs = append(l.regs, regPos{op: op, reg: reg, pos: l.stack})
	l.stack += size
}

func (l *layout) addSpecial(save, restore string, size int) {
	l.regs = append(l.regs, regPos{save: save, restore: restore, pos: l.stack})
	l.stack += size
}

func (l *layout) save() {
	for _, reg := range l.regs {
		if reg.save != "" {
			p(reg.save, reg.pos)
		} else {
			p("%s %s, %d(%s)", reg.op, reg.reg, reg.pos, l.sp)
		}
	}
}

func (l *layout) restore() {
	for i := len(l.regs) - 1; i >= 0; i-- {
		reg := l.regs[i]
		if reg.restore != "" {
			p(reg.restore, reg.pos)
		} else {
			p("%s %d(%s), %s", reg.op, reg.pos, l.sp, reg.reg)
		}
	}
}

func gen386() {
	p("PUSHFL")

	// Save general purpose registers.
	var l = layout{sp: "SP"}
	for _, reg := range regNames386 {
		if reg == "SP" || strings.HasPrefix(reg, "X") {
			continue
		}
		l.add("MOVL", reg, 4)
	}

	// Save the 387 state.
	l.addSpecial(
		"FSAVE %d(SP)\nFLDCW runtime·controlWord64(SB)",
		"FRSTOR %d(SP)",
		108)

	// Save SSE state only if supported.
	lSSE := layout{stack: l.stack, sp: "SP"}
	for i := 0; i < 8; i++ {
		lSSE.add("MOVUPS", fmt.Sprintf("X%d", i), 16)
	}

	p("ADJSP $%d", lSSE.stack)
	p("NOP SP")
	l.save()
	p("CMPB internal∕cpu·X86+const_offsetX86HasSSE2(SB), $1\nJNE nosse")
	lSSE.save()
	label("nosse:")
	p("CALL ·asyncPreempt2(SB)")
	p("CMPB internal∕cpu·X86+const_offsetX86HasSSE2(SB), $1\nJNE nosse2")
	lSSE.restore()
	label("nosse2:")
	l.restore()
	p("ADJSP $%d", -lSSE.stack)

	p("POPFL")
	p("RET")
}

func genAMD64() {
	// Assign stack offsets.
	var l = layout{sp: "SP"}
	for _, reg := range regNamesAMD64 {
		if reg == "SP" || reg == "BP" {
			continue
		}
		if strings.HasPrefix(reg, "X") {
			l.add("MOVUPS", reg, 16)
		} else {
			l.add("MOVQ", reg, 8)
		}
	}

	// TODO: MXCSR register?

	// Apparently, the signal handling code path in darwin kernel leaves
	// the upper bits of Y registers in a dirty state, which causes
	// many SSE operations (128-bit and narrower) become much slower.
	// Clear the upper bits to get to a clean state. See issue #37174.
	// It is safe here as Go code don't use the upper bits of Y registers.
	p("#ifdef GOOS_darwin")
	p("VZEROUPPER")
	p("#endif")

	p("PUSHQ BP")
	p("MOVQ SP, BP")
	p("// Save flags before clobbering them")
	p("PUSHFQ")
	p("// obj doesn't understand ADD/SUB on SP, but does understand ADJSP")
	p("ADJSP $%d", l.stack)
	p("// But vet doesn't know ADJSP, so suppress vet stack checking")
	p("NOP SP")
	l.save()
	p("CALL ·asyncPreempt2(SB)")
	l.restore()
	p("ADJSP $%d", -l.stack)
	p("POPFQ")
	p("POPQ BP")
	p("RET")
}

func genARM() {
	// Add integer registers R0-R12.
	// R13 (SP), R14 (LR), R15 (PC) are special and not saved here.
	var l = layout{sp: "R13", stack: 4} // add LR slot
	for i := 0; i <= 12; i++ {
		reg := fmt.Sprintf("R%d", i)
		if i == 10 {
			continue // R10 is g register, no need to save/restore
		}
		l.add("MOVW", reg, 4)
	}
	// Add flag register.
	l.addSpecial(
		"MOVW CPSR, R0\nMOVW R0, %d(R13)",
		"MOVW %d(R13), R0\nMOVW R0, CPSR",
		4)

	// Add floating point registers F0-F15 and flag register.
	var lfp = layout{stack: l.stack, sp: "R13"}
	lfp.addSpecial(
		"MOVW FPCR, R0\nMOVW R0, %d(R13)",
		"MOVW %d(R13), R0\nMOVW R0, FPCR",
		4)
	for i := 0; i <= 15; i++ {
		reg := fmt.Sprintf("F%d", i)
		lfp.add("MOVD", reg, 8)
	}

	p("MOVW.W R14, -%d(R13)", lfp.stack) // allocate frame, save LR
	l.save()
	p("MOVB ·goarm(SB), R0\nCMP $6, R0\nBLT nofp") // test goarm, and skip FP registers if goarm=5.
	lfp.save()
	label("nofp:")
	p("CALL ·asyncPreempt2(SB)")
	p("MOVB ·goarm(SB), R0\nCMP $6, R0\nBLT nofp2") // test goarm, and skip FP registers if goarm=5.
	lfp.restore()
	label("nofp2:")
	l.restore()

	p("MOVW %d(R13), R14", lfp.stack)     // sigctxt.pushCall pushes LR on stack, restore it
	p("MOVW.P %d(R13), R15", lfp.stack+4) // load PC, pop frame (including the space pushed by sigctxt.pushCall)
	p("UNDEF")                            // shouldn't get here
}

func genARM64() {
	// Add integer registers R0-R26
	// R27 (REGTMP), R28 (g), R29 (FP), R30 (LR), R31 (SP) are special
	// and not saved here.
	var l = layout{sp: "RSP", stack: 8} // add slot to save PC of interrupted instruction
	for i := 0; i <= 26; i++ {
		if i == 18 {
			continue // R18 is not used, skip
		}
		reg := fmt.Sprintf("R%d", i)
		l.add("MOVD", reg, 8)
	}
	// Add flag registers.
	l.addSpecial(
		"MOVD NZCV, R0\nMOVD R0, %d(RSP)",
		"MOVD %d(RSP), R0\nMOVD R0, NZCV",
		8)
	l.addSpecial(
		"MOVD FPSR, R0\nMOVD R0, %d(RSP)",
		"MOVD %d(RSP), R0\nMOVD R0, FPSR",
		8)
	// TODO: FPCR? I don't think we'll change it, so no need to save.
	// Add floating point registers F0-F31.
	for i := 0; i <= 31; i++ {
		reg := fmt.Sprintf("F%d", i)
		l.add("FMOVD", reg, 8)
	}
	if l.stack%16 != 0 {
		l.stack += 8 // SP needs 16-byte alignment
	}

	// allocate frame, save PC of interrupted instruction (in LR)
	p("MOVD R30, %d(RSP)", -l.stack)
	p("SUB $%d, RSP", l.stack)
	p("#ifdef GOOS_linux")
	p("MOVD R29, -8(RSP)") // save frame pointer (only used on Linux)
	p("SUB $8, RSP, R29")  // set up new frame pointer
	p("#endif")
	// On darwin, save the LR again after decrementing SP. We run the
	// signal handler on the G stack (as it doesn't support SA_ONSTACK),
	// so any writes below SP may be clobbered.
	p("#ifdef GOOS_darwin")
	p("MOVD R30, (RSP)")
	p("#endif")

	l.save()
	p("CALL ·asyncPreempt2(SB)")
	l.restore()

	p("MOVD %d(RSP), R30", l.stack) // sigctxt.pushCall has pushed LR (at interrupt) on stack, restore it
	p("#ifdef GOOS_linux")
	p("MOVD -8(RSP), R29") // restore frame pointer
	p("#endif")
	p("MOVD (RSP), R27")          // load PC to REGTMP
	p("ADD $%d, RSP", l.stack+16) // pop frame (including the space pushed by sigctxt.pushCall)
	p("JMP (R27)")
}

func genMIPS(_64bit bool) {
	mov := "MOVW"
	movf := "MOVF"
	add := "ADD"
	sub := "SUB"
	r28 := "R28"
	regsize := 4
	if _64bit {
		mov = "MOVV"
		movf = "MOVD"
		add = "ADDV"
		sub = "SUBV"
		r28 = "RSB"
		regsize = 8
	}

	// Add integer registers R1-R22, R24-R25, R28
	// R0 (zero), R23 (REGTMP), R29 (SP), R30 (g), R31 (LR) are special,
	// and not saved here. R26 and R27 are reserved by kernel and not used.
	var l = layout{sp: "R29", stack: regsize} // add slot to save PC of interrupted instruction (in LR)
	for i := 1; i <= 25; i++ {
		if i == 23 {
			continue // R23 is REGTMP
		}
		reg := fmt.Sprintf("R%d", i)
		l.add(mov, reg, regsize)
	}
	l.add(mov, r28, regsize)
	l.addSpecial(
		mov+" HI, R1\n"+mov+" R1, %d(R29)",
		mov+" %d(R29), R1\n"+mov+" R1, HI",
		regsize)
	l.addSpecial(
		mov+" LO, R1\n"+mov+" R1, %d(R29)",
		mov+" %d(R29), R1\n"+mov+" R1, LO",
		regsize)
	// Add floating point control/status register FCR31 (FCR0-FCR30 are irrelevant)
	l.addSpecial(
		mov+" FCR31, R1\n"+mov+" R1, %d(R29)",
		mov+" %d(R29), R1\n"+mov+" R1, FCR31",
		regsize)
	// Add floating point registers F0-F31.
	for i := 0; i <= 31; i++ {
		reg := fmt.Sprintf("F%d", i)
		l.add(movf, reg, regsize)
	}

	// allocate frame, save PC of interrupted instruction (in LR)
	p(mov+" R31, -%d(R29)", l.stack)
	p(sub+" $%d, R29", l.stack)

	l.save()
	p("CALL ·asyncPreempt2(SB)")
	l.restore()

	p(mov+" %d(R29), R31", l.stack)     // sigctxt.pushCall has pushed LR (at interrupt) on stack, restore it
	p(mov + " (R29), R23")              // load PC to REGTMP
	p(add+" $%d, R29", l.stack+regsize) // pop frame (including the space pushed by sigctxt.pushCall)
	p("JMP (R23)")
}

func genPPC64() {
	// Add integer registers R3-R29
	// R0 (zero), R1 (SP), R30 (g) are special and not saved here.
	// R2 (TOC pointer in PIC mode), R12 (function entry address in PIC mode) have been saved in sigctxt.pushCall.
	// R31 (REGTMP) will be saved manually.
	var l = layout{sp: "R1", stack: 32 + 8} // MinFrameSize on PPC64, plus one word for saving R31
	for i := 3; i <= 29; i++ {
		if i == 12 || i == 13 {
			// R12 has been saved in sigctxt.pushCall.
			// R13 is TLS pointer, not used by Go code. we must NOT
			// restore it, otherwise if we parked and resumed on a
			// different thread we'll mess up TLS addresses.
			continue
		}
		reg := fmt.Sprintf("R%d", i)
		l.add("MOVD", reg, 8)
	}
	l.addSpecial(
		"MOVW CR, R31\nMOVW R31, %d(R1)",
		"MOVW %d(R1), R31\nMOVFL R31, $0xff", // this is MOVW R31, CR
		8)                                    // CR is 4-byte wide, but just keep the alignment
	l.addSpecial(
		"MOVD XER, R31\nMOVD R31, %d(R1)",
		"MOVD %d(R1), R31\nMOVD R31, XER",
		8)
	// Add floating point registers F0-F31.
	for i := 0; i <= 31; i++ {
		reg := fmt.Sprintf("F%d", i)
		l.add("FMOVD", reg, 8)
	}
	// Add floating point control/status register FPSCR.
	l.addSpecial(
		"MOVFL FPSCR, F0\nFMOVD F0, %d(R1)",
		"FMOVD %d(R1), F0\nMOVFL F0, FPSCR",
		8)

	p("MOVD R31, -%d(R1)", l.stack-32) // save R31 first, we'll use R31 for saving LR
	p("MOVD LR, R31")
	p("MOVDU R31, -%d(R1)", l.stack) // allocate frame, save PC of interrupted instruction (in LR)

	l.save()
	p("CALL ·asyncPreempt2(SB)")
	l.restore()

	p("MOVD %d(R1), R31", l.stack) // sigctxt.pushCall has pushed LR, R2, R12 (at interrupt) on stack, restore them
	p("MOVD R31, LR")
	p("MOVD %d(R1), R2", l.stack+8)
	p("MOVD %d(R1), R12", l.stack+16)
	p("MOVD (R1), R31") // load PC to CTR
	p("MOVD R31, CTR")
	p("MOVD 32(R1), R31")        // restore R31
	p("ADD $%d, R1", l.stack+32) // pop frame (including the space pushed by sigctxt.pushCall)
	p("JMP (CTR)")
}

func genRISCV64() {
	p("// No async preemption on riscv64 - see issue 36711")
	p("UNDEF")
}

func genS390X() {
	// Add integer registers R0-R12
	// R13 (g), R14 (LR), R15 (SP) are special, and not saved here.
	// Saving R10 (REGTMP) is not necessary, but it is saved anyway.
	var l = layout{sp: "R15", stack: 16} // add slot to save PC of interrupted instruction and flags
	l.addSpecial(
		"STMG R0, R12, %d(R15)",
		"LMG %d(R15), R0, R12",
		13*8)
	// Add floating point registers F0-F31.
	for i := 0; i <= 15; i++ {
		reg := fmt.Sprintf("F%d", i)
		l.add("FMOVD", reg, 8)
	}

	// allocate frame, save PC of interrupted instruction (in LR) and flags (condition code)
	p("IPM R10") // save flags upfront, as ADD will clobber flags
	p("MOVD R14, -%d(R15)", l.stack)
	p("ADD $-%d, R15", l.stack)
	p("MOVW R10, 8(R15)") // save flags

	l.save()
	p("CALL ·asyncPreempt2(SB)")
	l.restore()

	p("MOVD %d(R15), R14", l.stack)    // sigctxt.pushCall has pushed LR (at interrupt) on stack, restore it
	p("ADD $%d, R15", l.stack+8)       // pop frame (including the space pushed by sigctxt.pushCall)
	p("MOVWZ -%d(R15), R10", l.stack)  // load flags to REGTMP
	p("TMLH R10, $(3<<12)")            // restore flags
	p("MOVD -%d(R15), R10", l.stack+8) // load PC to REGTMP
	p("JMP (R10)")
}

func genWasm() {
	p("// No async preemption on wasm")
	p("UNDEF")
}

func notImplemented() {
	p("// Not implemented yet")
	p("JMP ·abort(SB)")
}

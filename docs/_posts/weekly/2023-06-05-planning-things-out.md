---
layout: post
title:  "Getting a hang on things"
date:   2023-06-05 23:06:28 +0100
categories: weekly todo
---

{% comment %}
Week 23/06/05 report.

Copyright (C) 2023  Benjamin Priour <priour.be@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
{% endcomment %}

Welcome to the first real post of this blog !

I will most likely grow lazy, and keep the side talk to the bare minimum.
Though hopefully, I'll keep updating the **important** stuff, that is once a week summarize my progress on the analyzer.

### TLDR

---

<br>

## Tuesday

Today was the first day I could fully dedicate to GCC, and after discussing a bit on the IRC channels, @jwakely oriented me to some wonderful links stated below.

Looking at bugs [106390][PR106390] and [106391][PR106391], one can get an inkling at how extra attributes and annotations to C++ standard library could immensely help the analyzer.

Some good arguments are pointed out, and I plan on going down that road as early as possible, as I believe annotating the standard library is a cornerstone to any non-trivial progress towards supporting C++.

However, this will be a long and tedious undertaking.
So first, to get more familiar with the analyzer, I'm planning and adding support to the `operator new`

Running the analyzer on itself `make CXXFLAGS="-fanalyzer"`, we mostly get noise. Yet, by filtering and eliminating the redundant warnings the first coherent one that is **not** a `-Wanalyzer-use-of-uninitialized-value` is a `possible-null-dereference`.

> ../../gcc/gcc/make-unique.h:41:30: warning: use of possibly-NULL ‘operator new(120)’ where non-null expected [CWE-690] [-Wanalyzer-possible-null-argument]

This one is actually a **false positive**, that is a warning false emitted, on something that can actually never happen.
Why ? Because the `operator new` used here can throw, thus will never returns `NULL`.

#### <u>what to do</u>

1. Enhance the *known_function* operator new and its variants in the analyzer, so as to support **placement new**, throwing and non-throwing variants, as well as their delete counterparts.
> This might be done by looking at the `noexcept` qualifier of the operator called. If throwing, then null cannot be returned. Otherwise it can.

- Test and verify that the `new`/`delete` operators are suitably matched, that is a 'regular' new acquisition should not be released by a placement delete, nor by an array-wise delete.
> A placement new of size X should also be followed by a placement delete of size X

- Check that a `placement new` does not fall out of bounds of the memory chunk it is using. See bug [105948][PR105948].


## Wednesday & Thursday

I had patch https://gcc.gnu.org/git/gitweb.cgi?p=gcc.git;h=9589a46ddadc8b to bugs [109439][PR109439] and [109437][PR109437] taking dust in my backlog.
This fixes how a `-Wanalyzer-use-of-uninitialized-value` was always tagging along `-Wanalyzer-out-of-bounds`, although fixing the latter
would result in fixing the former.
I had already bootstrapped and regtested this bug about 10 days ago, and just had to rebase it and fix formatting issues.

## Friday

Unfortunately the above patch has resulted in a regression bug, filed as [110198][PR110198]. The delay between the push and the regtesting resulted in this bug appearing. Sadly, I will be offline next week, so I won't necessarily have time to fix it then.


[PR106390]: https://gcc.gnu.org/bugzilla/show_bug.cgi?id=106390
[PR106391]: https://gcc.gnu.org/bugzilla/show_bug.cgi?id=106391
[PR105948]: https://gcc.gnu.org/bugzilla/show_bug.cgi?id=105948
[PR109439]: https://gcc.gnu.org/bugzilla/show_bug.cgi?id=109439
[PR109437]: https://gcc.gnu.org/bugzilla/show_bug.cgi?id=109437
[PR110198]: https://gcc.gnu.org/bugzilla/show_bug.cgi?id=110198
# GSoC 23 - Extending gcc -fanalyzer C++ support for self-analysis

## Description

Currently, the static analyzer offers little dedicated support for C++. Even when projecting the most straightforward valid C test cases to C++, the checkers output incorrect diagnostics, either by their absence or imprecision.

The issues this project aims to tackle are all prerequisites to further extensions of C++ support. The aim of this project is to enable the analyzer to self-analyze itself.
It will be done through :

1. Recording false positives and false negatives from the self-analysis, as well as paths marked too complex to solve.

2. Tuning the analyzer parameters to draw a balance between self-analysis's completeness and build time.

3. Generalizing relevant gcc.dg/analyzer tests to be run with both C and C++.

4. Adding support for `-Wanalyser-double-free` to C++ allocation pairs `new`, `new[]`, `delete` and `delete[]` both throwing and non-throwing versions.

5. Improving the diagnostic paths for the standard library, by not diving into the standard library code.

### Expected deliverables

- [x] New test cases reflecting item 3.
- [x] A patch for item 4
- [x] A patch for item 5
- [ ] A patch fixing the analyzer true positives at the end of the project.

## What has been done

### Changes in the initial project goals

|Initial Goal|Change|Why|
| --- | --- | --- |
|Add support for tracking unique_ptr null-dereference| Aborted | I've spent a week designing a solution. The long-term answer was to implement a new state-machine to track ownership transfers in C++. It has been deemed to be a task requiring a whole summer by itself. |
|Add support for `Wanalyser-double-free` to C++ allocation pairs...|Exceeded|Not only support of `Wanalyzer-double-free` was added but of all warnings already in place for `malloc`. The initial project was oblivious of non-allocating `new` (i.e. *placement new*), yet they now are fully supported as well, and out-of-bounds accesses are detected. |
|Improve the emission of two diagnostics at the same statement|Additional|I found that two unrelated diagnostics - i.e. resolving one would not fix the other - could still supersede one another if emitted at the same statement, leading to false positives. |

### Patches delivered

- Full support of C++ operator new has been added with:
  [`analyzer: Add support of placement new and improved operator new [PR105948, PR94355]`](https://gcc.gnu.org/pipermail/gcc-patches/2023-August/629010.html)

- A new option to reduce the verbosity of the analyzer and prevent the emitted diagnostics
  to show system headers internal have been added with:
  [`analyzer: New option fanalyzer-show-events-in-system-headers [PR110543]`](https://gcc.gnu.org/pipermail/gcc-patches/2023-August/627142.html)

- Two warnings emitted by the analyzer could supersedes one another if emitted at the same statement, without any regards to the path that led to each of them. A patch has been written
so that two diagnostics resulting from incompatible paths can never supersedes each other.
  [`analyzer: Call off a superseding when diagnostics are unrelated [PR110830]`](https://gcc.gnu.org/pipermail/gcc-patches/2023-September/629501.html)

- gcc.dg/analyzer tests were previously only testing for C support and have been generalized to both C and C++ with the ongoing serie of patches:
 [`analyzer: Move gcc.dg/analyzer tests to c-c++-common (1) [PR96395]`](https://gcc.gnu.org/pipermail/gcc-patches/2023-August/628507.html)
 [`analyzer: Move gcc.dg/analyzer tests to c-c++-common (2) [PR96395]`](https://gcc.gnu.org/pipermail/gcc-patches/2023-September/629234.html)

### Patches in review

- Added an extra warning for C++ emitted when a class's field is named similarly to one inherited.
 [`c++: Additional warning for name-hiding [PR12341]`](https://gcc.gnu.org/pipermail/gcc-patches/2023-September/629233.html)

### Ongoing work

## What's left

### Future plans

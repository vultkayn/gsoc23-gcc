---
layout: post
title:  "Regression fix"
date:   2023-06-19 10:06:28 +0100
categories: weekly todo
---

{% comment %}
Week 23/06/19 report.

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

I'm back from last week vacations, and so have to fix regression [110198][PR110198] as soon as possible.
The mistake was actually just from a preemptive return, and I manage to track down why [100244][PR100244]
was failing pretty quickly.

However, my current box is quite short on CPU power, and a full regtesting cycle locks me out my laptop
for about 2 days - got 2 cores sitting at 2.4 Ghz. I've filed a request 10 days ago for an account on the
[compile farm](https://cfarm.tetaneutral.net/) that was accepted.
Unfortunately, my auth keys are taking a while to be shared among the machines, and I must stick with my laptop
in the meantime. As I believe fixing this regression takes priority, I will devote my computer to this task first.
That's basically 4 days off as I have to rebase my patch on a fresh trunk, trunk that I also have to regtest,
to later compare.

## Thursday

I gave up on regtesting using this computer and instead ordered a new one, much more powerful (14 cores at 5 Ghz).
I'll use the stipend from GSoC to finance that.
My old laptop was mostly locked as foreseen, so I used that time to set up my new machine, freshly unboxed.
The regression has been going just fine and I sent it to mail list for approval (see [email](mail))
I've met with my mentor David Malcolm through a video call, and we discussed my advancements and future tasks.

## Friday

Finally this week was used to write wide test cases for `operator new` as this only takes off very little CPU cycles,
and make a comprehensive list of the warnings emitted.
I've begun making a patch toward [supporting C++ new](operator new) in the analyzer, starting with the false positive
`-Wanalyzer-possible-null-dereference` given for throwing versions of new, even without `-fno-exceptions` - see [(1) to (4)](https://en.cppreference.com/w/cpp/memory/new/operator_new).

The analyzer itself uses *new expression*, but with `-fno-exceptions` enabled.
We do get **true positive** *-Wanalyzer-possible-null-dereference* warnings, but not exactly for the right reason.
The call was mistaken for a `non-throwing new`, instead of a more correct `throwing new with exceptions disabled`. 




[PR110198]: https://gcc.gnu.org/bugzilla/show_bug.cgi?id=110198
[PR100244]: https://gcc.gnu.org/bugzilla/show_bug.cgi?id=100244
[operator new]: https://gcc.gnu.org/bugzilla/show_bug.cgi?id=94355
[mail]: https://gcc.gnu.org/pipermail/gcc-patches/2023-June/623060.html

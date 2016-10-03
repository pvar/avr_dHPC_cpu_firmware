# nstBASIC

## Yet another... #NOT

This is yet another implementation of BASIC programming language
and though I was strongly tempted to call it yaBASIC, I managed
to resist the (timeworn) acronym joke. Instead, I went with another
acronym, that may not be funny at all, but succeeds on another level:
It tells the whole story.

This project is an improved version of TinyBasicPlus which, in turn,
improves upon [Tiny BASIC][tbasic]. And although the word 'improved' is loosely
used, keep in mind that this implementation has many differences from it's
predecessors in almost every aspect. It features an enriched command set and
the main parts of the program have been rewritten in a (hopefuylly) more
clear manner. It's not-so-tiny anymore and the name nstBASIC sprang from
exactly this realisation. Besides, keeping the word tiny in the name, helps
denote the heritage of the code.

## More than an interpreter

nstBASIC is written for an 8bit microcontroller, just like TinyBasicPlus,
but the code has been expanded in many ways, refactored and spread across
many files. You could rightly argue that making a larger version of an
otherwise complete project, cannot be considered as an improvement.
Nevertheless, such a claim, estranged from any reference to the purpose
of the project, would be misleading.

TinyBasicPlus was developed for the Arduino platform and the user can interact
with the language interpreter through a serial connection, using a terminal
emulator on his computer. nstBASIC is something totally different! It is only
meant to run on the specific hardware of a [homemade computer][dhpc], which
features a keyboard input, a sound card and a graphics card. Essentially,
apart from being a language interpreter, nstBASIC acts as the BIOS of the
aforementioned computer: It handles communication with the peripherals.

## (In)separable parts

nstBASIC runs on the CPU of my homemade computer, which is not a real CPU. It's an
ATMEGA644, that belogns to the family of 8bit AVRs. The specifics of this computer
will be discussed in the documentation of the relevant project. For now, keep in mind
that both the sound card and the graphics card are also implemented around 8bit
microcontrollers. The main idea here, is that nstBASIC can be seen as a piece of firmware.
It belongs to a set of programs that turn each microcontroller into a self-contained device.
At the same time, these programs are designed to work together and along with some
"glue logic" form a complete system. In other words, nstBASIC is an integral part
of this computer.

Don't let the above description discourage you, if you wish to use the interpreter
of BASIC in an entirely different project. The hardware dependent code is conveniently
isolated from the interpreter code. All functions that are "tied" to the hardware reside
in @c io.c. Some other functions that you may have to tweak (or even get rid of), are
those controlling the IO pins in @c cmd_pinctl.c. In most cases, in order to use the
interpreter in a different hardware setup, you'll only have to hack those two files
and a few constants in interpreter.h and printing.h. This convenient separation
of functionality could be seen as a by-product of refactoring the interpreter,
although in reality, it was a prerequisite. In TinyBasicPlus, the interpreter
was implemented in a huge function and the control flow was achieved by the abundant
use of goto command. The code was super-fast but almost impossible to study, extend
or improve. In order to remedy this situation, I created a bundle of functions and
a simple mechanism, that would allow the propagation of some signals -- previously
hosted in global variables. The first step to this direction was to group functions
in different files, according to the purpose each one served...

## License

As described above, the main part of nstBASIC derived from the extensive modifications
of TinyBasicPlus' core. The interpreter was practically written from scratch and the
general structure of the code was altered significantly. There are some functions though,
mainly those of the parser, which were only tweaked on a cosmetic level. The functions
that handle communication with the rest of the hardware were written by me.

TinyBasicPlus is distributed under MIT license. The code I wrote, as well as
the modified/extended interpreter, is distributed under the [GPLv3 Licence][GPL3].
The same applies for nstBASIC as a whole.

## Related links

- My homemade computer's [circuit][dhpc]. If you want to run nstBASIC on the real hardware
it was written for, you have to make it yourself!
- The [repository][tbasic] for TinyBasicPlus. This is where you'll find the mother-project,
from which nstBASIC was born.
- [deltaHacker][delta] is a great magazine. The homemade computer on which nstBASIC runs,
was built as an excuse to write lots of cool articles, for the said magazine.

[delta]:    http://deltahacker.gr                      "ethical hacking magazine"
[dhpc]:     https://github.com/pvar/dhpc_hardware      "schematics and PCB"
[tbasic]:   https://github.com/BleuLlama/TinyBasicPlus "source code repository"
[GPL3]:     www.gnu.org/licenses/gpl-3.0.html          "official license text"

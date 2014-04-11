RamseyX Client
====
RamseyX Client is the client program of distributed computing project RamseyX.
It is designed to help DC volunteers contribute their computing power to the project more easily.

INSTALL
----
Do this to perform a standard install:
```Bash
git clone https://github.com/RamseyX/RamseyX.git && \ # Grab the Source
cd RamseyX && git checkout Qt && \ # Switch to Branch Qt
qmake && make && sudo make install # Compile and install
```

You need `boost`, `Qt4` or up and `curl` installed. <br />
For the autoupdate function, have `p7zip` ready. (Well, actually we don't have that function for non-Windows platforms.... This breaks things like package managers...)

*Tired of building this yourself? Download the pre-build binary (and maybe wine it)!*

**WINDOWS USERS**: Change the `RamseyX.pro` file according to your actual boost and curl path. Or use Qt IDEs.

COPYING
----
Copyright (C) 2013-2014 Zizheng Tai <zizheng.tai@gmail.com>, et al.<br />
**For full lists, see [GitHub](https://github.com/RamseyX/RamseyX/graphs/contributors).**

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this program.
If not, see <http://www.gnu.org/licenses/>.

CONTACT
----
If you have problems, questions, ideas or suggestions,
please contact us by posting an email to a suitable email address listed
in the "About RamseyX Client" part in the program.

WEBSITE
----
Visit our website for the latest news and downloads:

<http://www.ramseyx.org/>

GIT
----
To download the very latest source off the GIT server do this:
```Bash
git clone https://github.com/RamseyX/RamseyX.git
```
(you'll get a directory named RamseyX created, filled with the source code)

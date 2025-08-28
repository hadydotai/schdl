# schdl

A small tool that displays a day schedule blocked by slots of time. Built using
[raylib](https://www.raylib.com/) and C.

![Screenshot](screenshot.png)

## Building

```sh
cd deps
./DEPS
cd ..
make install-deps && make && make install
```

## Running

```sh
schdl
```

You'll need to have a directory with `.schedule` files inside. With the
following format:

```
Meeting with John: 09:00 - 10:00.
Work on project X: 10:00 - 13:00.
Break: 1:00pm - 02:30 pm.
```

All these lines are valid but must end with `.`, you can use time formats in 12
or 24 hours. Schedule files are loaded according to the week day, so they need
to be named like `sunday.schedule`, `monday.schedule`, etc.

# License

schdl Copyright (C) 2025 hadydotai

This program is free software: you can redistribute it and/or modify it under
the terms of the GNU Affero General Public License as published by the Free
Software Foundation, either version 3 of the License, or any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE. See the GNU Affero General Public License for more details.

You should have received a copy of the GNU Affero General Public License along
with this program. If not, see <http://www.gnu.org/licenses/>.

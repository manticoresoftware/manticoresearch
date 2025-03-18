# Component licenses

Manticore Search uses several open-source software components, each governed by
its respective copyright and license terms. This document provides an overview of
the components.

## Manticore Search

Copyright:
```
Copyright (c) 2017-2025, Manticore Software LTD (https://manticoresearch.com)
All rights reserved
```
Manticore Search is released under [GPL v3 or later](https://raw.githubusercontent.com/manticoresoftware/manticoresearch/master/LICENSE).

Manticore Search redistributes or is based on the following open source software:

## Sphinx Search

https://sphinxsearch.com/ , https://github.com/sphinxsearch/sphinx/tree/e93969e20920737a24ad9b55651bb9b72fd81174

Source files: all files in the project that have Sphinx's copyright in them.

Copyright:
```
Copyright (c) 2001-2016, Andrew Aksyonoff
Copyright (c) 2008-2016, Sphinx Technologies Inc
All rights reserved
```

License: [GPL-2.0 or later](./sphinx-license).

### NOTICE:
Manticore Search is based on [Sphinx Search](https://github.com/sphinxsearch/sphinx/tree/e93969e20920737a24ad9b55651bb9b72fd81174) and was forked from the commit
available at https://github.com/sphinxsearch/sphinx/commit/e93969e20920737a24ad9b55651bb9b72fd81174.
The corresponding commit on Manticore Search's side is available at
https://github.com/manticoresoftware/manticoresearch/commit/e93969e20920737a24ad9b55651bb9b72fd81174.
As stated in the documentation of the version of Sphinx Search Manticore Search
is based on (https://github.com/sphinxsearch/sphinx/blob/e93969e20920737a24ad9b55651bb9b72fd81174/doc/sphinx.txt#L630):

> This program is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the Free
Software Foundation; either version 2 of the License, or (at your option)
any later version.

No source file of Sphinx Search that is used, modified, or redistributed by
Manticore Search contradicts this. None of them specify a specific GPL license
version.

The GPL licenses (versions 1 and 2) state that:

> If the Program does not specify a version number of this License, you may choose
any version ever published by the Free Software Foundation.

Based on these terms, Manticore Search, a fork of Sphinx Search, is modified and
redistributed under the later version of the GNU General Public License (version 3).

## Manticore Columnar Lirary

https://github.com/manticoresoftware/columnar

Source files: Manticore Search project does not modify of redistribute any source
files from this component. However, its source files are utilized during the
compilation process and become a part of Manticore Search binaries. Additionally,
Manticore Search binaries may dynamically load and utilize the library at runtime.

Copyright:
```
Copyright (c) 2020-2025, Manticore Software LTD (https://manticoresearch.com)
All rights reserved
```

License: [Apache 2.0](https://raw.githubusercontent.com/manticoresoftware/columnar/master/LICENSE)

## libsphinxclient

Source files: `./api/libsphinxclient/*`

Copyright:
```
Copyright (c) 2001-2016, Andrew Aksyonoff
Copyright (c) 2008-2016, Sphinx Technologies Inc
Copyright (c) 2017-2025, Manticore Software LTD (https://manticoresearch.com)
All rights reserved

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU Library General Public License. You should
have received a copy of the LGPL license along with this program; if you
did not, you can find it at http://www.gnu.org/
```

License: [LGPLv3](../api/libsphinxclient/COPYING) or later.

## old ruby client
Source files: `./api/ruby/*`

Copyrights:
```
lib/sphinx/client.rb:# Copyright:: Copyright (c) 2006 - 2008 Dmytro Shteflyuk
spec/fixtures/sphinxapi.php:// Copyright (c) 2001-2016, Andrew Aksyonoff
spec/fixtures/sphinxapi.php:// Copyright (c) 2008-2016, Sphinx Technologies Inc
```

License:
```
This library is distributed under the terms of the Ruby license.
You can freely distribute/modify this library.
```

## old php client
Source files: `./api/sphinxapi.php`

Copyright:
```
Copyright (c) 2001-2016, Andrew Aksyonoff
Copyright (c) 2008-2016, Sphinx Technologies Inc
Copyright (c) 2017-2025, Manticore Software LTD (https://manticoresearch.com)
All rights reserved

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU Library General Public License. You should
have received a copy of the LGPL license along with this program; if you
did not, you can find it at http://www.gnu.org/
```

License: [LGPL-3.0](../api/lgpl-3.0.txt).

## cJSON
https://github.com/DaveGamble/cJSON

Source files:
- `./src/json/cJSON.c`
- `./src/json/cJSON.h`

Copyright:
```
Copyright (c) 2009-2017 Dave Gamble and cJSON contributors

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
```

License: [MIT License](./cjson-license).

## libmysqlclient
https://dev.mysql.com/doc/relnotes/connector-cpp/en/

Source files: Manticore Search project does not modify of redistribute any source
files from this component. However, its source files are utilized during the
compilation process and become a part of Manticore Search binaries. Additionally,
Manticore Search binaries may dynamically load and utilize the library at runtime.

Copyright:
```
Copyright (c) 2000, 2023, Oracle and/or its affiliates.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License, version 2.0,
as published by the Free Software Foundation.

This program is also distributed with certain software (including
but not limited to OpenSSL) that is licensed under separate terms,
as designated in a particular file or component or in included license
documentation.  The authors of MySQL hereby grant you an additional
permission to link the program and your derivative works with the
separately licensed software that they have included with MySQL.

Without limiting anything contained in the foregoing, this file,
which is part of C Driver for MySQL (Connector/C), is also subject to the
Universal FOSS Exception, version 1.0, a copy of which can be found at
http://oss.oracle.com/licenses/universal-foss-exception.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License, version 2.0, for more details.

You should have received a copy of the GNU General Public License
```

License: [GPLv2 with FOSS Exception](./libmysqlclient-license).

## ./cmake/FindMysql.cmake
Source files:
- `./cmake/FindMysql.cmake`

Copyrights:
```
Copyright (c) 2008, 2010, Oracle and/or its affiliates. All rights reserved.

The MySQL Connector/C++ is licensed under the terms of the GPLv2
<http://www.gnu.org/licenses/old-licenses/gpl-2.0.html>, like most
MySQL Connectors. There are special exceptions to the terms and
conditions of the GPLv2 as it is applied to this software, see the
FLOSS License Exception
<http://www.mysql.com/about/legal/licensing/foss-exception.html>.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published
by the Free Software Foundation; version 2 of the License.

This program is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA
=============================================================================
Copyright 2015 Sphinx Technologies, Inc.
Copyright 2017-2025, Manticore Software LTD (https://manticoresearch.com)

Distributed under the OSI-approved BSD License (the "License");
see accompanying file Copyright.txt for details.

This software is distributed WITHOUT ANY WARRANTY; without even the
implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the License for more information.
```

Licenses:
- [GPLv2 with FOSS Exception](./FindMysql.cmake-license-gplv2floss)
- [OSI-approved BSD License](../cmake/FindMysql.cmake)

## http_parser
Source files:
- `./src/http/http_parser.h`, `./src/http/http_parser.c`

Copyright:
```
Copyright Joyent, Inc. and other Node contributors. All rights reserved.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to
deal in the Software without restriction, including without limitation the
rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
sell copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
IN THE SOFTWARE.
```

License: [MIT License](./http_parser-license).

## cmake
https://cmake.org/

Source files: Manticore Search project does not modify of redistribute any source
files from this component. CMake is used to build Manticore Search packages.

Copyright:
```
Copyright 2000-2024 Kitware, Inc. and Contributors
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:

* Redistributions of source code must retain the above copyright
  notice, this list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright
  notice, this list of conditions and the following disclaimer in the
  documentation and/or other materials provided with the distribution.

* Neither the name of Kitware, Inc. nor the names of Contributors
  may be used to endorse or promote products derived from this
  software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
```

License: https://cmake.org/licensing/ .

## clang
https://clang.llvm.org/

Source files: Manticore Search project does not modify of redistribute any source
files from this component. CLang is used to build Manticore Search packages.

Copyright:
```
The LLVM project does not collect copyright assignments, which means that the copyright for the code in the project is held by the respective contributors. Because you (or your company) retain ownership of the code you contribute, you know it may only be used under the terms of the open source license you contributed it under: the license for your contributions cannot be changed in the future without your approval.

Because the LLVM project does not require copyright assignments, changing the LLVM license requires tracking down the contributors to LLVM and getting them to agree that a license change is acceptable for their contributions. We feel that a high burden for relicensing is good for the project, because contributors do not have to fear that their code will be used in a way with which they disagree.
```

License: [Apache License v2.0 with LLVM Exceptions](https://raw.githubusercontent.com/llvm/llvm-project/main/LICENSE.TXT).

## lz4
Source files: `./src/lz4/*`

Copyright:
```
Copyright (C) 2011-present, Yann Collet.

BSD 2-Clause License (http://www.opensource.org/licenses/bsd-license.php)

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

    * Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above
copyright notice, this list of conditions and the following disclaimer
in the documentation and/or other materials provided with the
distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

You can contact the author at :
 - LZ4 homepage : http://www.lz4.org
 - LZ4 source repository : https://github.com/lz4/lz4
```

License: [BSD 2-Clause License](./lz4-license).

## libstemmer
https://snowballstem.org/

Source files: Manticore Search project does not modify of redistribute any source
files from this component. However, its source files are utilized during the
compilation process and become a part of Manticore Search binaries. Additionally,
Manticore Search binaries may dynamically load and utilize the library at runtime.

Copyright:
```
Copyright (c) 2001, Dr Martin Porter,
Copyright (c) 2002, Richard Boulton.
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
```

License: [BSD 3-Clause License](https://snowballstem.org/license.html).

## Ninja
https://ninja-build.org/ , https://github.com/ninja-build/ninja

Source files: Manticore Search project does not modify of redistribute any source
files from this component. Ninja binaries are used to build Manticore Search
packages.

Copyright:
```
Copyright 2011 Google Inc. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

   http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
```

License: [Apache 2.0](https://raw.githubusercontent.com/ninja-build/ninja/master/COPYING).

## RE2
https://github.com/google/re2

Source files: Manticore Search project does not modify of redistribute any source
files from this component. However, its source files are utilized during the
compilation process and become a part of Manticore Search binaries. Additionally,
Manticore Search binaries may dynamically load and utilize the library at runtime.

Copyright:
```
Copyright 2023 The RE2 Authors.  All Rights Reserved.
Use of this source code is governed by a BSD-style
```

License: [BSD 3-Clause License](https://raw.githubusercontent.com/google/re2/main/LICENSE).

## ICU
https://github.com/unicode-org/icu/

Source files: Manticore Search project does not modify of redistribute any source
files from this component. However, its source files are utilized during the
compilation process and become a part of Manticore Search binaries. Additionally,
Manticore Search binaries may dynamically load and utilize the library at runtime.

Copyright:
```
Copyright © 2016-2023 Unicode, Inc. Unicode and the Unicode Logo are registered trademarks of Unicode, Inc. in the United States and other countries.
```

License: [UNICODE LICENSE V3](https://raw.githubusercontent.com/unicode-org/icu/main/LICENSE).

## JSON for Modern C++
https://github.com/nlohmann/json

Source files: Manticore Search project does not modify of redistribute any source
files from this component. However, its source files are utilized during the
compilation process and become a part of Manticore Search binaries.

Copyright:
```
Copyright © 2013-2022 Niels Lohmann

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the “Software”), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
```

License: [MIT](https://opensource.org/license/mit/).

## Bison
https://www.gnu.org/software/bison/

Source files: Manticore Search project does not modify of redistribute any source
files from Bison. However, Bison's source files are utilized during the
compilation process.

Copyright:
```
A Bison parser, made by GNU Bison 3.8.2.

Bison interface for Yacc-like parsers in C

Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2021 Free Software Foundation,
Inc.

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

As a special exception, you may create a larger work that contains
part or all of the Bison parser skeleton and distribute that work
under terms of your choice, so long as that work isn't itself a
parser generator using the skeleton or a modified version thereof
as a parser skeleton.  Alternatively, if you modify or redistribute
the parser skeleton itself, you may (at your option) remove this
special exception, which will cause the skeleton and the resulting
Bison output files to be licensed under the GNU General Public
License without this special exception.

This special exception was added by the Free Software Foundation in
version 2.2 of Bison.  

DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
especially those whose name start with YY_ or yy_.  They are
private implementation details that can be changed or removed.
```

License: [GPLv3](https://www.gnu.org/software/bison/).

## libzstd
https://github.com/facebook/zstd

Source files: Manticore Search project does not modify of redistribute any source
files from this component. However, its source files are utilized during the
compilation process and become a part of Manticore Search binaries. Additionally,
Manticore Search binaries may dynamically load and utilize the library at runtime.

Copyright:
```
Copyright (c) Meta Platforms, Inc. and affiliates. All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

 * Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

 * Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

 * Neither the name Facebook, nor Meta, nor the names of its contributors may
   be used to endorse or promote products derived from this software without
   specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
```

License: [BSD License](https://raw.githubusercontent.com/facebook/zstd/dev/LICENSE).

## libssl
https://www.openssl.org/ , https://github.com/openssl/openssl

Source files: Manticore Search project does not modify of redistribute any source
files from this component. However, its source files are utilized during the
compilation process and become a part of Manticore Search binaries. Additionally,
Manticore Search binaries may dynamically load and utilize the library at runtime.

Copyright:
```
Copyright (c) 1998-2023 The OpenSSL Project Authors

Copyright (c) 1995-1998 Eric A. Young, Tim J. Hudson

All rights reserved.
```

License: [Apache 2.0](https://raw.githubusercontent.com/openssl/openssl/master/LICENSE.txt).

## libpq
http://www.postgresql.org/ , https://github.com/postgres/postgres

Source files: Manticore Search project does not modify of redistribute any source
files from this component. However, its source files are utilized during the
compilation process and become a part of Manticore Search binaries. Additionally,
Manticore Search binaries may dynamically load and utilize the library at runtime.

Copyright:
```
PostgreSQL Database Management System
(formerly known as Postgres, then as Postgres95)

Portions Copyright (c) 1996-2024, PostgreSQL Global Development Group

Portions Copyright (c) 1994, The Regents of the University of California

Permission to use, copy, modify, and distribute this software and its
documentation for any purpose, without fee, and without a written agreement
is hereby granted, provided that the above copyright notice and this
paragraph and the following two paragraphs appear in all copies.

IN NO EVENT SHALL THE UNIVERSITY OF CALIFORNIA BE LIABLE TO ANY PARTY FOR
DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING
LOST PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS
DOCUMENTATION, EVEN IF THE UNIVERSITY OF CALIFORNIA HAS BEEN ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.

THE UNIVERSITY OF CALIFORNIA SPECIFICALLY DISCLAIMS ANY WARRANTIES,
INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
AND FITNESS FOR A PARTICULAR PURPOSE.  THE SOFTWARE PROVIDED HEREUNDER IS
ON AN "AS IS" BASIS, AND THE UNIVERSITY OF CALIFORNIA HAS NO OBLIGATIONS TO
PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
```

License: [PostgreSQL License](https://www.postgresql.org/about/licence/).

## libexpat
https://libexpat.github.io/

Source files: Manticore Search project does not modify of redistribute any source
files from this component. However, its source files are utilized during the
compilation process and become a part of Manticore Search binaries. Additionally,
Manticore Search binaries may dynamically load and utilize the library at runtime.

Copyright:
```
Copyright (c) 1998-2000 Thai Open Source Software Center Ltd and Clark Cooper
Copyright (c) 2001-2022 Expat maintainers

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
```

License: [MIT License](https://raw.githubusercontent.com/libexpat/libexpat/master/COPYING).

## unixodbc
https://www.unixodbc.org/

Source files: Manticore Search project does not modify of redistribute any source
files from this component. However, its source files are utilized during the
compilation process and become a part of Manticore Search binaries. Additionally,
Manticore Search binaries may dynamically load and utilize the library at runtime.

Copyright:
```
* Created by Nick Gorham
* (nick@lurcher.org).
*
* copyright (c) 1999 Nick Gorham
*
* This library is free software; you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public
* License as published by the Free Software Foundation; either
* version 2 of the License, or (at your option) any later version.
*
* This library is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with this library; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
```

License: LGPL-2.1+ :
```
This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.
.
This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.
.
You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
.
On Debian GNU/Linux systems, the complete text of the GNU Lesser General
Public License can be found in `/usr/share/common-licenses/LGPL-2.1'.
```

## Galera library

https://galeracluster.com/library/ , https://github.com/codership/galera

Manticore Search is capable of utilizing the Galera library for replication, but
this is not done by default when you install a Manticore Search package. The
integration of the Galera library is entirely at the user's discretion and depends
on whether they install an additional package that provides the library. It should
be noted that Manticore Search does not use, modify, or redistribute any of Galera's
source files, including library header files, at any stage of building, packaging,
or distributing Manticore Search binaries.

License: [GPLv2](https://raw.githubusercontent.com/codership/galera/3.x/LICENSE).

## xxHash

https://xxhash.com/, https://github.com/Cyan4973/xxHash

Source files: Manticore Search project uses a [modified version](https://github.com/manticoresoftware/xxhash)
of xxHash, but does not redistribute any of its source files. However, these source files
are utilized during the compilation process and become part of the Manticore
Search binaries.

Copyright:
```
 * Copyright (C) 2012-2023 Yann Collet
 *
 * BSD 2-Clause License (https://www.opensource.org/licenses/bsd-license.php)
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *    * Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *    * Redistributions in binary form must reproduce the above
 *      copyright notice, this list of conditions and the following disclaimer
 *      in the documentation and/or other materials provided with the
 *      distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * You can contact the author at:
 *   - xxHash homepage: https://www.xxhash.com
 *   - xxHash source repository: https://github.com/Cyan4973/xxHash
```

License: [BSD 2-Clause License](https://www.opensource.org/licenses/bsd-license.php)

## cppjieba

https://github.com/yanyiwu/cppjieba

Source files: Manticore Search project uses a [modified version](https://github.com/manticoresoftware/cppjieba)
of cppjieba, but does not redistribute any of its source files. However, these
source files are utilized during the compilation process and become part of the
Manticore Search binaries.

License: [MIT License](https://opensource.org/license/mit)

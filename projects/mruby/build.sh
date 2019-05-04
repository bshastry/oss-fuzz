#!/bin/bash -eu
# Copyright 2018 Google Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
################################################################################

# build project
export LD=clang
export LDFLAGS="$CFLAGS"
./minirake clean && ./minirake -j$(nproc) all

# build fuzzers
FUZZ_TARGET=$SRC/mruby/oss-fuzz/mruby_fuzzer.c
name=$(basename $FUZZ_TARGET .c)
$CC -c $CFLAGS -Iinclude \
     ${FUZZ_TARGET} -o $OUT/${name}.o
$CXX $CXXFLAGS $OUT/${name}.o $LIB_FUZZING_ENGINE -lm \
    $SRC/mruby/build/host/lib/libmruby.a -o $OUT/${name}
rm -f $OUT/${name}.o

# dict and config
cp $SRC/mruby/oss-fuzz/config/mruby.dict $OUT
cp $SRC/mruby/oss-fuzz/config/mruby_fuzzer.options $OUT

# seeds
find $SRC/mruby_seeds -exec zip -ujq \
    $OUT/mruby_fuzzer_seed_corpus.zip "{}" \;

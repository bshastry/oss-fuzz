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

# compile LPM
(mkdir LPM && cd LPM && cmake $SRC/libprotobuf-mutator -GNinja -DLIB_PROTO_MUTATOR_DOWNLOAD_PROTOBUF=ON -DLIB_PROTO_MUTATOR_TESTING=OFF -DCMAKE_BUILD_TYPE=Release && ninja)

# build project
export LD=clang
export LDFLAGS="$CFLAGS"
(cd $SRC/mruby; patch -p1 < $SRC/print.patch; ./minirake clean && ./minirake -j$(nproc) all)

rm -rf genfiles && mkdir genfiles && LPM/external.protobuf/bin/protoc mruby_bytecode.proto --cpp_out=genfiles
LPM/external.protobuf/bin/protoc ruby.proto --cpp_out=genfiles

# build MRB fuzzer harness
$CXX $CXXFLAGS mruby_proto_fuzzer.cpp genfiles/mruby_bytecode.pb.cc \
  -I genfiles -I.  -I libprotobuf-mutator/  -I LPM/external.protobuf/include \
  -I mruby/include -lz -lm \
  LPM/src/libfuzzer/libprotobuf-mutator-libfuzzer.a \
  LPM/src/libprotobuf-mutator.a \
  LPM/external.protobuf/lib/libprotobuf.a \
  mruby/build/host/lib/libmruby.a \
  $LIB_FUZZING_ENGINE \
  -o $OUT/mruby_bytecode_proto_fuzzer

# build ruby fuzzer
$CXX $CXXFLAGS mrubylang_proto_fuzzer.cpp genfiles/ruby.pb.cc proto_to_ruby.cpp \
  -I genfiles -I.  -I libprotobuf-mutator/  -I LPM/external.protobuf/include \
  -I mruby/include -lz -lm \
  LPM/src/libfuzzer/libprotobuf-mutator-libfuzzer.a \
  LPM/src/libprotobuf-mutator.a \
  LPM/external.protobuf/lib/libprotobuf.a \
  mruby/build/host/lib/libmruby.a \
  $LIB_FUZZING_ENGINE \
  -o $OUT/mrubylang_bytecode_proto_fuzzer

# build ruby fuzzer with debug info
$CXX $CXXFLAGS -DJUSTDOIT mrubylang_proto_fuzzer.cpp genfiles/ruby.pb.cc proto_to_ruby.cpp \
  -I genfiles -I.  -I libprotobuf-mutator/  -I LPM/external.protobuf/include \
  -I mruby/include -lz -lm \
  LPM/src/libfuzzer/libprotobuf-mutator-libfuzzer.a \
  LPM/src/libprotobuf-mutator.a \
  LPM/external.protobuf/lib/libprotobuf.a \
  mruby/build/host/lib/libmruby.a \
  $LIB_FUZZING_ENGINE \
  -o $OUT/mrubylang_bytecode_proto_fuzzer_debug

## Build corpus based fuzzer
for file in $SRC/mruby*.c; do
name=$(basename $file .c)
$CC -c $CFLAGS -I$SRC/mruby/include \
     $file -o $OUT/${name}.o
$CXX $CXXFLAGS $OUT/${name}.o -lFuzzingEngine -lm \
    $SRC/mruby/build/host/lib/libmruby.a -o $OUT/${name}
rm -f $OUT/${name}.o
done

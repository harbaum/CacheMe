#!/bin/bash
SRC=`find . -name *.cpp`
lupdate $SRC -ts -no-obsolete ./lang/cacheme_de.ts
lupdate $SRC -ts -no-obsolete ./lang/cacheme_sk.ts
lupdate $SRC -ts -no-obsolete ./lang/cacheme_fi.ts
lupdate $SRC -ts -no-obsolete ./lang/cacheme_cs.ts
lupdate $SRC -ts -no-obsolete ./lang/cacheme_sv.ts
lupdate $SRC -ts -no-obsolete ./lang/cacheme_xx.ts

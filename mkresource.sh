#!/bin/bash
# script to create a qt resource file used to embed resources
# into the binary
NAME=cacheme.qrc

echo "<!DOCTYPE RCC><RCC version=\"1.0\">" > $NAME
echo "<qresource>" >> $NAME
for i in data/icons/*.svg ; do 
    BASENAME=`basename $i`
    echo "<file alias=\"$BASENAME\">$i</file>" >> $NAME
done
echo "</qresource>" >> $NAME
echo "</RCC>" >> $NAME
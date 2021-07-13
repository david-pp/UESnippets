#!/bin/sh

if [ $# -eq 0 ]; then
    echo "Usage1 : $0 Dst  -  Clone MyBlankProgram to Dst"
    echo "Usage2 : $0 Src Dst - Clone Src to Dst"
    exit 1
fi

SNIPPET_FROM=MyBlankProgram

if [ $# -eq 1 ]; then
    SNIPPET_TO=$1
elif [ $# -eq 2 ]; then
    SNIPPET_FROM=$1
    SNIPPET_TO=$2
fi

echo ">>>>>>> Clone ... $SNIPPET_FROM -> $SNIPPET_TO"

rm -rf $SNIPPET_TO/
cp -Rf $SNIPPET_FROM $SNIPPET_TO/

# Rename
for file in `find $SNIPPET_TO/`; do
    newfile=${file/${SNIPPET_FROM}/${SNIPPET_TO}} 
    if [ "$file" != "$newfile" ]; then
        echo ">> $file -> $newfile"
        mv -v $file $newfile

        # Replace Content
        if [ "$file" != "${SNIPPET_TO}//Resources/Windows/MyBlankProgram.ico" ]; then
            sed -i "" "s/${SNIPPET_FROM}/${SNIPPET_TO}/g" $newfile 
        fi
    fi
done


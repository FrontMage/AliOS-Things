#! /bin/sh

CNN=CnnKernels\*
EXTRA=ExtraKernels\*
FACEDET=FaceDetKernels\*
GEN=Gen\*

FILES=$(find . -type f -iname "$GEN" -o -iname "$CNN" -o -iname "$EXTRA" -o -iname "$FACEDET")

for file in $FILES
do
    echo $file
done

echo "Delete generated files ?"

rm --interactive=once $FILES

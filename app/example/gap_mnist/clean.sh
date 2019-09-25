#! /bin/sh

MNIST=MnistKernels\*
GEN=Gen\*

FILES=$(find . -type f -iname "$GEN" -o -iname "$MNIST")

for file in $FILES
do
    echo $file
done

echo "Delete generated files ?"

rm --interactive=once $FILES

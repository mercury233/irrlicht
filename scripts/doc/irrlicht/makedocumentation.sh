mkdir -p ../../../doctemp/html

rm tut.txt || true;
for i in ../../../examples/[012]*/main.cpp; do
  sed -f tutorials.sed $i >>tut.txt;
done

# Enable for latex docs
#doxygen doxygen-pdf.cfg

# Enable for html docs
doxygen doxygen.cfg

#cp doxygen.css ../../../doctemp/html
cp irrlicht.png logobig.png ../../../doctemp/html

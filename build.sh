make clean
make
cd $PLUGINS_SRC
for d in */; do
	cd "$d"
		make
		cd ..
done

make clean
make
cd $BCIRC_PLUGINS_SRC
for d in */; do
	cd "$d"
		make
		cd ..
done

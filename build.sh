make
cd plugins
for d in */; do
	cd "$d"
		make
		cd ..
done

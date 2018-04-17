for i in {3..96}; do
	for j in {3..96}; do
		grep "Open -> ($i,$j)" test.txt | wc -l
	done
done

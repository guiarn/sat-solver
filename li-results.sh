#!/bin/sh

# Small script to gather data from picosat and your own SAT solver
# Useful for LI prac 01
#
# Example usage:
#   ./li-results.sh > out.txt

# Your solver
BIN=./solver

info() {
	echo "$@" >&2
}

write_results() {
	local name="$1"
	local raw="$2"

	SATI=$(echo "$raw" | sed -n '/^s /s/s \([^ ]\+\).*/\1/p')
	TIME=$(echo "$raw" | sed -n 's/^c \(.*\) total run time$/\1/p')
	NDEC=$(echo "$raw" | sed -n 's/^c \(.*\) decisions$/\1/p')
	NPPS=$(echo "$raw" | sed -n 's/^c \(.*props.*\)$/\1/p')

	echo "## $name"
	echo "$SATI"
	echo "Total time: $TIME"
	echo "Total number of decisions: $NDEC"
	echo "Propagations per second: $NPPS"
	echo 
}

for f in $(ls tests/*.cnf); do
	echo "$f"
	echo "========================"
	echo 

	info "picosat -v < $f"
	write_results "Picosat" "$(picosat -v < $f)"

	info "$BIN < $f"
	write_results "Own SAT solver" "$($BIN < $f)"
done

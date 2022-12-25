start: program echoer

program: main.cpp
	g++ -o program main.cpp

echoer: 
	@echo "# Funfair Prepayment Simulation"
	@echo "# Running format: ./program <input_file>"

clean:
	rm -f program
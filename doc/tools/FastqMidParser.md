### FastqMidParser tool help
	FastqMidParser (0.1-420-g3536bb0)
	
	Counts the number of occurances of each MID in a FASTQ file.
	
	Mandatory parameters:
	  -in <file>    Input gzipped FASTQ file.
	
	Optional parameters:
	  -out <file>   Output TXT file. If unset, writes to STDOUT.
	                Default value: ''
	  -lines <int>  The number of FASTQ entries in the input file to parse. 0 is unlimited.
	                Default value: '1000'
	  -mids <int>   The number of top-ranking MIDs to print. 0 is unlimited.
	                Default value: '20'
	  -sheet <file> Optional sample sheet CSV file as provided to CASAVA. If given, the closest match in the sample sheet is printed after each MID.
	                Default value: ''
	
	Special parameters:
	  --help        Shows this help and exits.
	  --version     Prints version and exits.
	  --changelog   Prints changeloge and exits.
	  --tdx         Writes a Tool Definition Xml file. The file name is the application name with the suffix '.tdx'.
	
### FastqMidParser changelog
	FastqMidParser 0.1-420-g3536bb0
	
[back to ngs-bits](https://github.com/imgag/ngs-bits)
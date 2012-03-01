// Wesley Hymnal Processor's main program file
// Copyright 2012 by Sam Cantrell

#include <iostream>
#include <string>
#include <fstream>
#include <boost/algorithm/string.hpp>

using namespace std;
using namespace boost::algorithm;

#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1

// FUNCTIONS
// Display usage options for program.
void usage();
// Check to see if given file exists.
bool fileExists(const string fileName);
// Open hymn file and process it, adding it to the output,
// and then closing it.
int processHymn(string hymnFileName, string outputFileName);
// Open output, insert page break, and then close output
// file.
int insertNewPage(string outputFileName);

// PROGRAM BEGINS HERE
int main(int argc, char* argv[])
{
	// VARIABLE DECLARATION
	// Variable for the input hymnal file name
	string hymnalFileName;
	// Variable for the output ABC file name
	string outputFileName;
	// File handle for input file
	ifstream hymnalData;
	// Variable for grabbing a line of data from the hymnal input file
	// to process
	string lineOfData;

	// Check for number of arguments
	if (argc < 4) {
		usage();
		return EXIT_FAILURE;
	}
	else {
		// TODO: Insert true command line processing
		hymnalFileName = argv[1];
		outputFileName = argv[3];
	}

	// Attempt to open hymnal data file
	hymnalData.open(hymnalFileName.c_str());
	if (!hymnalData)
	{
		// File not found or successfully opened, abort.
		cout << "Error: " << hymnalFileName << " not found." << endl;
		return EXIT_FAILURE;
	}

	// Check to see if the file listed for output already
	// exists. If it does, abort.
	if (fileExists(outputFileName) == true)
	{
		// File found, so abort.
		cout << "Error: Output file already exists." << endl;
		return EXIT_FAILURE;
	}

	// Now begin parsing the file
	// Right now hymns aren't sorted by their numbers, but rather inserted
	// in the order they are listed in the WHY (Wesley HYmnal) file. This
	// means that the user has to order them correctly.
	while(getline(hymnalData, lineOfData))
	{
		// Get each line of data from hymnal file. Right now, that
		// should only be either a %%NEWPAGE code or a hymn file name.

		// Check for %%NEWPAGE code
		if (lineOfData == "%%NEWPAGE")
		{
			if (insertNewPage(outputFileName) != EXIT_SUCCESS)
			{
				// Returned with error condition
				cout << "Error: Unable to open " << outputFileName << endl;
				return EXIT_FAILURE;
			}
		}
		else
		{
			// Must be a hymn file name
			if (processHymn(lineOfData, outputFileName) != EXIT_SUCCESS)
			{
				// There was an error processing this hymn
				return EXIT_FAILURE;
			}
		}
	}

	// Close hymnal input file, as we're done
	hymnalData.close();

	return EXIT_SUCCESS;
}

int processHymn(string hymnFileName, string outputFileName)
{
	string data;

	ifstream hymnData;
	ofstream outputData;

	// Information fields for hymn
	// Would this work better as a struct?
	string title;
	string composer;
	string lyricist;
	string meter;
	string tune = "Unknown";
	string time;

	// TODO: Presently category and scripture are not loaded.
	string category;
	string scripture;

	string abcData;

	string staffSkip;
	int no = 0;
	float scale = 0.00;
	// End of declaring information fields


	// Open hymn file passed in
	hymnData.open(hymnFileName.c_str());
	if (!hymnData)
	{
		// File not found or opened successfully, abort.
		// We print the filename so that the user may
		// know which specific hymn caused the program
		// failure.
		cout << "Error: " << hymnFileName << " not opened successfully." << endl;
		return EXIT_FAILURE;
	}

	// Notify user that hymn was opened successfully.
	cout << hymnFileName << " opened successfully." << endl;
	cout << "Parsing hymn..." << endl;
	hymnData >> data;
	while (!hymnData.eof())
	{
		if (data == "%%NUMBER")
		{
			hymnData >> no;
		}
		else if (data == "%%TITLE")
		{
			char tempData[99];
			hymnData.getline(tempData, 99, '\n');

			title = tempData;
			trim(title);
		}
		else if (data == "%%COMPOSER")
		{
			char tempData[99];
			hymnData.getline(tempData, 99, '\n');

			composer = tempData;
			trim(composer);
		}
		else if (data == "%%LYRICIST")
		{
			char tempData[99];
			hymnData.getline(tempData, 99, '\n');

			lyricist = tempData;
			trim(lyricist);
		}
		else if (data == "%%METER")
		{
			char tempData[100];
			hymnData.getline(tempData, 99, '\n');

			meter = tempData;
			// Check for DOS-style line ending and
			// remove if found.
			if (meter[meter.length() - 1] == '\r')
			{
				meter = meter.substr(0, meter.length() - 1);
			}
		}
		else if (data == "%%TUNE")
		{
			char tempData[100];
			hymnData.getline(tempData, 99, '\n');

			tune = tempData;
			// Check for DOS-style line ending and
			// remove if found.
			if (tune[tune.length() - 1] == '\r')
			{
				tune = tune.substr(0, tune.length() - 1);
			}
		}
		else if (data == "%%TIMESIGNATURE")
		{
			hymnData >> time;
		}
		else if (data == "%%SCALE")
		{
			hymnData >> scale;
		}
		else if (data == "%%STAFFSKIP")
		{
			hymnData >> staffSkip;
		}
		else if (data == "%%BEGIN")
		{
			// This was a little finicky. I wasn't sure how to abort
			// out of the if statement and then the while so that
			// I could run a separate loop to grab all the ABC data,
			// so instead I embedded the loop in here. This is very
			// hacky!
			// TODO: Fix this later!
			while (!hymnData.eof())
			{
				char line[300];
				hymnData.getline(line, 300, '\n');
				if (line != "") {
					abcData = abcData + line + "\n";
				}
			}
		}
		hymnData >> data;
	}

	// While the following is useful for verification, and may prove
	// useful in some other fashion, disable for now.
	/*
	cout << "Number: " << no << endl;
	cout << "Title: " << title << endl;
	cout << "Lyricist: " << lyricist << endl;
	cout << "Composer: " << composer << endl;
	cout << "Tune: " << tune << endl;
	cout << "Meter: " << meter << endl;
	cout << "Category: " << category << endl;
	cout << "Scripture: " << scripture << endl;
	cout << "Time Signature: " << time << endl;
	cout << "Scale: " << scale << endl;
	cout << "Staff skip: " << staffSkip << endl;
	*/

	// Close hymn file
	hymnData.close();

	// Create output
	outputData.open(outputFileName.c_str(), ios::out | ios::app);

	if (!outputData)
	{
		// Output file not successfully opened; abort.
		cout << "Error: Output file not successfully opened." << endl;
		return EXIT_FAILURE;
	}

	// Create format information such as scale and staff skip amounts
	if (scale != 0) {
		outputData << "%%scale " << scale << endl;
	}

	if (staffSkip != "") {
		outputData << "%%staffskip " << staffSkip << endl;
	}

	// Put in the footer format; TODO: Add header info
	outputData << "%%footer \" 		" << tune << "\\n		" << meter << "\"" << endl;

	// Output the information fields
	outputData << "X: " << no << endl;
	outputData << "T: " << title << endl;
	outputData << "H: " << lyricist << endl;
	outputData << "C: " << composer << endl;
	outputData << "M: " << time;

	// Send the actual ABC data
	outputData << abcData << endl;
	outputData << endl << endl;

	// Close output file
	outputData.close();
	return EXIT_SUCCESS;
}

int insertNewPage(string outputFileName)
{
	// This function inserts a new page command
	ofstream outputData;

	outputData.open(outputFileName.c_str(), ios::out | ios::app);
	if (outputData)
	{
		outputData << endl << endl;
		outputData << "%%newpage" << endl;
		outputData << endl << endl;

		outputData.close();
	}
	else {
		// File not successfully opened
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

void usage()
{
	// Displays information on how to run the program.
	cout << "USAGE: wesley FILE -o OUTPUTFILE" << endl;
	cout << "Process the Wesley Hymnal FILE to create ABC file OUTPUTFILE." << endl;
}

bool fileExists(const string fileName)
{
	// Return true if fileName exists, otherwise
	// return false.
	ifstream ifile(fileName.c_str());
	return ifile;
}

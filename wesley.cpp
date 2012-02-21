// Wesley Hymnal Processor's main program file
// Copyright 2012 by Sam Cantrell

#include <iostream>
#include <string>
#include <fstream>

using namespace std;

int processHymn(string hymnFileName, string outputFileName);
int insertNewPage(string outputFileName);

int main(int argc, char* argv[])
{
	//if (argc != 2) {
	//	std::cout << "Incorrect number of arguments." << endl;
	//	return 1;
	//}

	ifstream hymnalData;
	char hymnalFileName[999];
	char outputFileName[999];
	
	string lineOfData;

	//cout << "Please enter hymn file name: ";
	//cin >> hymnFileName;

	// hymnFileName = argv[1];

	cout << "Please enter hymnal file name: ";
	cin >> hymnalFileName;
	
	cout << "Please enter the output file name: ";
	cin >> outputFileName;
	
	// Attempt to open hymnal data file
	hymnalData.open(hymnalFileName);
	if (!hymnalData)
	{
		// File not found or successfully opened, abort.
		cout << "Error: " << hymnalFileName << " not found." << endl;
		return 1;
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
			insertNewPage(outputFileName);
		}
		else
		{
			// Must be a hymn file name
			
			// TODO: Check to see if hymn was aborted
			int successful = processHymn(lineOfData, outputFileName);
		}
	}
	
	// Close hymnal input file, as we're done
	hymnalData.close();
		
	return 0;
}

int processHymn(string hymnFileName, string outputFileName)
{
	string data;
	
	ifstream hymnData;
	ofstream outputData;

	// Information fields for hymn
	// Would this work better as a struct?
	char title[100];
	char composer[100];
	char lyricist[100];
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
		// We print the hymn file name involved as this will
		// eventually be used in a batch process where hundreds of
		// files will be processed, and the user will want to know
		// which file was not found.
		cout << "Error: " << hymnFileName << " not found." << endl;
		return 1;
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
			hymnData.getline(title, 99, '\n');
		}
		else if (data == "%%COMPOSER")
		{
			hymnData.getline(composer, 99, '\n');
		}
		else if (data == "%%LYRICIST")
		{
			hymnData.getline(lyricist, 99, '\n');
		}
		else if (data == "%%METER")
		{
			char tempData[100];
			hymnData.getline(tempData, 99, '\n');
			
			meter = tempData;
			meter = meter.substr(0, meter.length() - 1);
		}
		else if (data == "%%TUNE")
		{
			char tempData[100];
			hymnData.getline(tempData, 99, '\n');
			
			tune = tempData;
			tune = tune.substr(0, tune.length() - 1);
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
	// As we're appending, any existing files will have this
	// data tacked on. We should probably eventually check to
	// see if the output file exists first.
	outputData.open(outputFileName.c_str(), ios::out | ios::app);
	
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
	return 0;
}

int insertNewPage(string outputFileName)
{
	// This function inserts a new page command
	ofstream outputData;
	
	outputData.open(outputFileName.c_str(), ios::out | ios::app);
	outputData << endl << endl;
	outputData << "%%newpage" << endl;
	outputData << endl << endl;
	
	outputData.close();
	
	return 0;
}

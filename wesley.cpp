// Wesley Hymnal Processor's main program file
// Copyright 2012 by Sam Cantrell

#include <iostream>
#include <string>
#include <fstream>

using namespace std;

int main(int argc, char* argv[])
{
	//if (argc != 2) {
	//	std::cout << "Incorrect number of arguments." << endl;
	//	return 1;
	//}

	char hymnFileName[999];
	ifstream hymnData;
	ofstream outputData;

	string data;

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

	cout << "Please enter hymn file name: ";
	cin >> hymnFileName;

	// hymnFileName = argv[1];

	hymnData.open(hymnFileName);
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
					cout << line << endl;
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
	outputData.open("output.abc");
	
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
	
	// Close output file
	outputData.close();
	
	return 0;
}

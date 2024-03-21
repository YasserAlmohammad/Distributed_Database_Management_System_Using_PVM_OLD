#ifndef _DB_H
#define _DB_H

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/*
	this class represents a database object, it references the database file and 
	do the main functionality a database required to do

	the database file has the following structure:

	<DATABASE>  DBName
		<METADATA>   # we can use this for statistcs
			<TABLE_NAMES> 
				names
			</TABLE_NAMES>
		</METADATA>

		<TABLES>
			<TABLE>
				<COLUMNS>
					<COLUMN> name datatype constraint </COLUMN>
					.
					.
					.
					.
				</COLUMNS>

				<ROWS>
					<ROW> col1data col2data col3data ...</ROW>
					.
					.
					.
					.

				</ROWS>
			</TABLE>

			<TABLE>

			</TABLE>
			.
			.
			.
			.
		</TABLES>


	</DATABASE>

	-   this notation help to ease reading the DB file, also to 
		encapsulate data inside tags
	-   datatypes allowed are: string int double
	-	constraint allowed are: PK, FK, UK(unique)

*/

class DB
{
public:
	DB(char* filePath,char* name); //database name and datafile path to create a new dataBase
	DB(){};
	static DB* createFromFile(char* filePath);
	~DB(void);
	
	//a one can create a new empty table then insert columns
	//this way you can modify the table strucure later without modifying data
	bool createTable(char* name); //false if table already exsists
	bool createCol(char* tableName,char* colName,char* dataType,char* constraint/*UK|PK|NULL*/);
private:
	FILE* dbFile;
	char* dbName;
	char* filePath;
	bool error;
	char* errorCode;
	bool connected;
public:
	//ckeck if a word is inside this line
	static int contains(const char* line, char* word,char* sep=", \t\n"/* separator list */);
	static char* getWord(const char* line,int index,char* sep=", \t\n"); //index starts from 1
	static bool checkType(char* cell,char* type); //check when inserting data to match data defined by column datatype
	static bool checkData2Col(char* datatypes,char* rowData);
	static bool checkConstraints(char* row ,char* newRow,char* datatypes);

	bool insertRow(char* table,char* data);
	char* getTableNames();
	char* getTableDefinition(char* tableName);
	char* getTableData(char* tableName);
	bool dropTable(char* tableName);
	bool deleteRow(char* tableName,int id);
	inline bool isConnected(){ return connected;}
	inline void connectDB(){ connected=true;}
	inline void disConnectDB(){ connected=false;}
	inline char* getErrCode(){return errorCode;}
	inline bool hasError(){ return error;}
	char* getDBName();
	/*  simple SELECT operation follows the form:
		
		SELECT col1Name,col2Name.. FROM OneTableName
		[WHERE coli <|>|= val]

		SELECT * FROM TableName
		[WHERE coli >|<|= val]

		- where clause is optional
		- only one table the select is applied on
		- one column specified in the condition clause
		- use , to separate the column list items
		- the string is checked for error in the same time the data is retrieved
		- no SPACE arround the operation
	*/
	char* select(char* colList,char* tableName,char* Con);
	bool checkCond(char* cond,char* colNames,char* row);
	char* query(char* queryStr);
	//cond: colName >|<|= val
	//colNames: colName datatype..
	//row: <ROW> col1Data...</ROW>
};

#endif

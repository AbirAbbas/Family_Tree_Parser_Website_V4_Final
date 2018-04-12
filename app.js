'use strict'

// C library API
const ffi = require('ffi');

// Express App (Routes)
const express = require("express");
const app     = express();
const path    = require("path");
const fileUpload = require('express-fileupload');

const mysql = require('mysql');

let connection = null;

app.use(fileUpload());

// Minimization
const fs = require('fs');
const JavaScriptObfuscator = require('javascript-obfuscator');

// Important, pass in port as in `npm run dev 1234`, do not change
const portNum = process.argv[2];

let sharedLib = ffi.Library('./libparser.so', {
  'GEDCOMtoIndiJSON': [ 'string', ['string'] ],		//return type first, argument list second
  'GEDCOMtoObjJSON': [ 'string', ['string'] ],		//return type first, argument list second
  'addIndi': [ 'string', ['string', 'string'] ],		//return type first, argument list second
  'createAndWriteGEDCOM': [ 'string', ['string', 'string'] ],	
  'getDescJson': [ 'string', ['string', 'string', 'string', 'string'] ],
  'getAncesJson': [ 'string', ['string', 'string', 'string', 'string'] ],
});

app.get('/connect', function(req, res) {
	connection = mysql.createConnection({ host     : 'dursley.socs.uoguelph.ca',  user     : req.query.username,  password : req.query.password,  database : req.query.databasename }); 
	connection.connect(function(err) {
        if(err) {
            res.send(err);
            return;
        }
		else {
			res.send({"message":"success"});
		}
	});
});

// Send HTML at root, do not change
app.get('/',function(req,res){
	res.sendFile(path.join(__dirname+'/public/index.html'));
});

app.get('/storeDB', function(req, res) {
	
	if (connection == null) {
		res.send({"message":"login"});
	}
	else {
		var queryString = "create table FILE (file_id int AUTO_INCREMENT primary key, file_Name VARCHAR(20) NOT NULL, source VARCHAR(250) NOT NULL, version VARCHAR(10) NOT NULL, encoding VARCHAR(10) NOT NULL, sub_name VARCHAR(62) NOT NULL, sub_addr VARCHAR(256), num_individuals INT, num_families INT)";	
		
		//tables were created
		connection.query("show tables like \'FILE\'", function (err, rows, fields) {
			if (JSON.stringify(rows) == '[]') {
				connection.query(queryString);
			} 
			else {
				connection.query('DELETE from FILE');
			}
			
			queryString = "create table INDIVIDUAL (ind_id INT AUTO_INCREMENT PRIMARY KEY, surname VARCHAR(256) NOT NULL, given_name VARCHAR(256) NOT NULL, sex VARCHAR(1), fam_size INT, source_file INT, FOREIGN KEY(source_file) REFERENCES FILE(file_id) ON DELETE CASCADE)";	
		
			//tables were created
			connection.query("show tables like \'INDIVIDUAL\'", function (err, rows, fields) {
				if (JSON.stringify(rows) == '[]') {
					connection.query(queryString);
				} 
				else {
					connection.query('DELETE from INDIVIDUAL');
					connection.query('DELETE from FILE');
				}
				
				connection.query('show tables', function (err, rows, fields) {
					console.log(rows);
				});
				
				//insert data into tables here
				//creates object with all required data
				var files = new Object();
				files.fileList = fs.readdirSync('uploads/');
				files.indiList = Array(files.fileList.length);
				files.objList = Array(files.fileList.length);
				for (var i = 0; i < files['fileList'].length ; i++) {
					files['indiList'][i] = JSON.parse(sharedLib.GEDCOMtoIndiJSON('./uploads/' + files['fileList'][i]));
					files['objList'][i] = JSON.parse(sharedLib.GEDCOMtoObjJSON('./uploads/' + files['fileList'][i]));
				}
				
				
				//insert objects here
				
				for (var i = 0, counter = 1; i < files['fileList'].length; i++) { 
					if (files['objList'][i].length == 0) {
						continue;
					}
					
					
					if (files['objList'][i][0].submitteraddress == '') {
						files['objList'][i][0].submitteraddress = 'NULL';
					}
		
					var insertQuery = "INSERT INTO FILE (file_id, file_Name, source, version, encoding, sub_name, sub_addr, num_individuals, num_families) VALUES ("+ 
					counter+", \'"+ files['fileList'][i]+"\', \'"+ files['objList'][i][0].source+ "\', \'"+ files['objList'][i][0].gedc+ "\', \'"+ files['objList'][i][0].encoding+ 
					"\', \'"+ files['objList'][i][0].submittername+ "\', \'"+ files['objList'][i][0].submitteraddress+ 
					"\', "+ files['objList'][i][0].indilength+ ", "+ files['objList'][i][0].famlength+ ")";
					
					connection.query(insertQuery);
					
					counter++;
				}
				
				for (var i = 0, counter = 1; i < files['fileList'].length; i++) { 
					if (files['objList'][i].length == 0) {
						continue;
					}
					
					for (var j = 0; j < files['indiList'][i].length; j++) {
						var insertQuery = "INSERT INTO INDIVIDUAL (surname, given_name, sex, fam_size, source_file) VALUES ('"+ files['indiList'][i][j].surname +"', '"+ files['indiList'][i][j].givenName +"', '"+ files['indiList'][i][j].sex +"', '"+ files['indiList'][i][j].familysize +"', "+ counter +")";
						connection.query(insertQuery);
					}
					
					counter++;
				}
				
				res.send({"message":"success"});
				
			});		
		});	
	}	
	
});

app.get('/clearDB', function(req, res) {
	if (connection == null) {
		res.send({"message":"login"});
	}
	else {
		connection.query("show tables like \'FILE\'", function (err, rows, fields) {
			
			if (JSON.stringify(rows) == '[]') {
				res.send({'message':'exist'});
			} 
			else {
				connection.query("show tables like \'INDIVIDUAL\'", function (err, rows, fields) {
					if (JSON.stringify(rows) == '[]') {
						res.send({'message':'exist'});
					} 
					else {
						connection.query('DELETE from INDIVIDUAL');
						connection.query('DELETE from FILE');
						res.send({'message':'success'});
					}
				});
			}
		});
	}
});

app.get('/displayDB', function(req, res) {
	if (connection == null) {
		res.send({"message":"login"});
	}
	else {
		connection.query("show tables like \'FILE\'", function (err, rows, fields) {
			if (JSON.stringify(rows) == '[]') {
				res.send({'message':'exist'});
			} 
			else {
				connection.query("show tables like \'INDIVIDUAL\'", function (err, rows, fields) {
					if (JSON.stringify(rows) == '[]') {
						res.send({'message':'exist'});
					} 
					else {
						//display here
						
						connection.query("select count(*) from FILE", function(err, rows, fields) {
							var filecount = rows[0]['count(*)'];
							connection.query("select count(*) from INDIVIDUAL", function(err, rows, fields) {
								var indicount = rows[0]['count(*)'];
								res.send({'message':'Database has '+filecount+' files and '+indicount+' individuals'});
							});
						});
						
					}
				});
			}
		});
	}
});

// Send Style, do not change
app.get('/style.css',function(req,res){
  //Feel free to change the contents of style.css to prettify your Web app
  res.sendFile(path.join(__dirname+'/public/style.css'));
});

app.get('/checkIfConneceted', function(req, res) {
	if (connection == null) {
		res.send({'message':'fail'});
	}
	else {
		res.send({'message':'success'});
	}
});

// Send obfuscated JS, do not change
app.get('/index.js',function(req,res){
  fs.readFile(path.join(__dirname+'/public/index.js'), 'utf8', function(err, contents) {
    const minimizedContents = JavaScriptObfuscator.obfuscate(contents, {compact: true, controlFlowFlattening: true});
    res.contentType('application/javascript');
    res.send(minimizedContents._obfuscatedCode);
  });
});

//Respond to POST requests that upload files to uploads/ directory
app.post('/upload', function(req, res) {
  if(!req.files) {
    return res.status(400).send('No files were uploaded.');
  }
 
  let uploadFile = req.files.uploadFile;
 
  // Use the mv() method to place the file somewhere on your server
	if (uploadFile.name.includes('.ged')) {
		uploadFile.mv('uploads/' + uploadFile.name, function(err) {
		if(err) {
		  return res.status(500).send(err);
		}

		res.redirect('/');
		});
	}
	else {
		res.status(500).send('error');
	}
});

//Respond to GET requests for files in the uploads/ directory
app.get('/uploads/:name', function(req , res){
  fs.stat('uploads/' + req.params.name, function(err, stat) {
    if(err == null) {
      res.sendFile(path.join(__dirname+'/uploads/' + req.params.name));
    } else {
      res.send('');
    }
  });
});


//******************** Your code goes here ******************** 

//Sample endpoint
app.get('/fileList', function(req , res){
	var files = new Object();
	files.fileList = fs.readdirSync('uploads/');
	files.indiList = Array(files.fileList.length);
	files.objList = Array(files.fileList.length);
	for (var i = 0; i < files['fileList'].length ; i++) {
		files['indiList'][i] = JSON.parse(sharedLib.GEDCOMtoIndiJSON('./uploads/' + files['fileList'][i]));
		files['objList'][i] = JSON.parse(sharedLib.GEDCOMtoObjJSON('./uploads/' + files['fileList'][i]));
	}

	res.send(files);
});

app.get('/indiSingle', function(req , res){
	var files = JSON.parse(sharedLib.GEDCOMtoIndiJSON('./uploads/' + req.query.fileName));
	res.send(files);
});

app.get('/background.jpg',function(req,res){
  res.sendFile(path.join(__dirname+'/public/background.jpg'));
});

app.get('/icon.png',function(req,res){
  res.sendFile(path.join(__dirname+'/public/icon.png'));
});

app.get('/createGEDCOM',function(req,res){
	var returnMessage = sharedLib.createAndWriteGEDCOM(JSON.stringify(req.query.obj), './uploads/' + req.query.fileName);
	
	var files = new Object();
	
	files.fileList = fs.readdirSync('uploads/');
	files.indiList = Array(files.fileList.length);
	files.objList = Array(files.fileList.length);
	for (var i = 0; i < files['fileList'].length ; i++) { 
		files['indiList'][i] = JSON.parse(sharedLib.GEDCOMtoIndiJSON('./uploads/' + files['fileList'][i]));
		files['objList'][i] = JSON.parse(sharedLib.GEDCOMtoObjJSON('./uploads/' + files['fileList'][i]));
	}

	res.send(files);
	
});

app.get('/addIndi', function(req , res){	
	sharedLib.addIndi(req.query.indi, './uploads/' + req.query.fileName)
	var files = JSON.parse(sharedLib.GEDCOMtoIndiJSON('./uploads/' + req.query.fileName));
	res.send(files);
});

app.get('/getDesc', function(req , res){	
	res.send(sharedLib.getDescJson('./uploads/' + req.query.fileName, req.query.givenName, req.query.surname, req.query.maxDepth));
});

app.get('/getAnces', function(req , res){	
	res.send(sharedLib.getAncesJson('./uploads/' + req.query.fileName, req.query.givenName, req.query.surname, req.query.maxDepth));
});

app.get('/totalFiles', function(req, res) {
	var fileList = fs.readdirSync('uploads/');
	
	res.send(fileList);
});

app.get('/customQuery', function(req, res) {
	var command = req.query.command;
	
	if (connection == null) {
		res.send({"message":"login"});
	}
	else {
		connection.query(command, function (err, rows, fields) {
			if (err) {
				res.send(err);
			}
			else {
				res.send(rows);
			}
		});
	}
});

app.listen(portNum);
console.log('Running app at localhost: ' + portNum);

var moment = require('moment');

function CreateReplace(table, body, fields) {
	try {
		var query = 'REPLACE INTO ' + table + ' VALUES(';
		var first = true;
		var args = [];
		
		for(var idx in fields) {
			if(first) {
				first = false;
			} else {
				query += ',';
			}
			
			query += '?';
		
			var entry = fields[idx];
			if(entry.type === 12) {
				try {
					var d = new moment(body[entry.name]);
					
					if(d.isValid()) {
						args.push(d.format('YYYY-MM-DD HH:mm:ss'));
					} else {
						args.push(null);
					}
				} catch(ex) {
					args.push(null);
				}
			} else {
				args.push(body[entry.name]);
			}
		}
		
		query += ')';
		
		return { 'query': query, 'args': args };
	} catch(ex) {
		return { 'query': '', 'args': [] };
	}
}

function CreateUpdate(req, res, table, pkey) {
	req.mysql.getConnection(function(err, connection) {
		try {
			if(err) {
				console.log(err);
				connection.release();
				res.sendStatus(500);
				return;
			}
			
			if(req.body[pkey] !== parseInt(req.params[pkey], 10)) {
				connection.release();
				res.sendStatus(400);
				return;
			}
			
			connection.query('SELECT * FROM ' + table + ' WHERE ' + pkey + '=? LIMIT 1', [req.params[pkey]], function (error, results, fields) {
				try {
					if(error) {
						console.log(error);
						connection.release();
						res.sendStatus(400);
						return;
					}
					
					var replace = CreateReplace(table, req.body, fields);
					if(replace.query === '') {
						connection.release();
						res.sendStatus(400);
						return;
					}
					
					connection.query(replace.query, replace.args, function(error, results, fields) {
						try {
							if(error) {
								console.log(error);
								connection.release();
								res.sendStatus(400);
								return;
							}
							
							connection.release();
							res.sendStatus(200);
						} catch(ex) {
							console.log(ex);
							connection.release();
							res.sendStatus(500);
						}
					});
				} catch(ex) {
					console.log(ex);
					connection.release();
					res.sendStatus(500);
				}
			});
		} catch(ex) {
			console.log(ex);
			connection.release();
			res.sendStatus(500);
		}
	});
}

function Retrieve(req, res, table, pkey) {
	req.mysql.getConnection(function(err, connection) {
		try {
			if(err) {
				console.log(err);
				connection.release();
				res.sendStatus(500);
				return;
			}
			
			connection.query('SELECT * FROM ' + table + ' WHERE ' + pkey + '=? LIMIT 1', [req.params[pkey]], function (error, results, fields) {
				try {
					if(results.length == 0) {
						connection.release();
						res.sendStatus(404);
						return;
					}
					
					var result = results[0];
					var ret = { };
					
					for(var idx in result) {
						var value = result[idx];
						ret[idx] = value;
					}
					
					connection.release();
					res.json(ret);
				} catch(ex) {
					console.log(ex);
					connection.release();
					res.sendStatus(500);
				}
			});
		} catch(ex) {
			console.log(ex);
			connection.release();
			res.sendStatus(500);
		}
	});
}

function Delete(req, res, table, pkey) {
	req.mysql.getConnection(function(err, connection) {
		try {
			if(err) {
				console.log(err);
				connection.release();
				res.sendStatus(500);
				return;
			}
						
			connection.query('DELETE FROM ' + table + ' WHERE ' + pkey + '=? LIMIT 1', [req.params[pkey]], function (error, results, fields) {
				try {
					if(error) {
						console.log(error);
						connection.release();
						res.sendStatus(400);
						return;
					}
					
					connection.release();
					res.sendStatus(200);
				} catch(ex) {
					console.log(ex);
					connection.release();
					res.sendStatus(500);
				}
			});
		} catch(ex) {
			console.log(ex);
			connection.release();
			res.sendStatus(500);
		}
	});
}

function Search(req, res, table, pkey) {
	//Verify incoming model
	if(!req.body.hasOwnProperty('draw')) {
		res.sendStatus(400);
		return;
	}
	
	if(!req.body.hasOwnProperty('start')) {
		res.sendStatus(400);
		return;
	}
	
	if(!req.body.hasOwnProperty('length')) {
		res.sendStatus(400);
		return;
	}
	
	if(!req.body.hasOwnProperty('search')) {
		res.sendStatus(400);
		return;
	}
	
	if(!req.body.hasOwnProperty('columns')) {
		res.sendStatus(400);
		return;
	}
	
	if(!req.body.hasOwnProperty('order')) {
		res.sendStatus(400);
		return;
	}

	req.mysql.getConnection(function(err, connection) {
		try {
			if(err) {
				console.log(err);
				connection.release();
				res.sendStatus(500);
				return;
			}
			
			var ret = { };
			ret.draw = req.body['draw'];
			ret.data = [];
			
			var query = 'SELECT ';
			var idx;
			var args = [];
			var first = true;
			for(idx in req.body['columns']) {
				var column = req.body['columns'][idx];
				if(first) {
					first = false;
				} else {
					query += ', ';
				}
				
				query += connection.escapeId(column.data);
			}
			
			query += ' FROM ' + table;
			
			first = true;
			for(idx in req.body['order']) {
				var order = req.body['order'][idx];
				if(first) {
					query += ' ORDER BY ';
					first = false;
				} else {
					query += ', ';
				}
				
				var column = req.body['columns'][order.column];
				
				query += connection.escapeId(column.data);
				
				if(order.dir === 'asc') {
					query += ' ASC';
				} else {
					query += ' DESC';
				}
			}
						
			connection.query(query, args, function (error, results, fields) {
				try {
					if(error) {
						console.log(error);
						connection.release();
						res.sendStatus(400);
						return;
					}
					
					ret.recordsTotal = results.length;
							
					for(var result_idx in results) {
						var result = results[result_idx];
						if(req.body['search'].value && req.body['search'].value.length > 0) {
							var found = false;
							
							for(idx in req.body['columns']) {
								var column = req.body['columns'][idx];
								
								if(column.searchable) {
									if(String(result[column.data]).toLowerCase().includes(String(req.body['search'].value).toLowerCase())) {
										found = true;
										break;
									}
								}
							}
							
							if(found) {
								var obj = { };
					
								for(var i in result) {
									var value = result[i];
									obj[i] = value;
								}
							
								ret.data.push(obj);
							}
						} else {
							var obj = { };
					
							for(var i in result) {
								var value = result[i];
								obj[i] = value;
							}
							
							ret.data.push(obj);
						}
					}
					
					ret.recordsFiltered = ret.data.length;				
					ret.data = ret.data.slice(req.body['start'], req.body['start'] + req.body['length']);
					
					connection.release();
					res.json(ret);
				} catch(ex) {
					console.log(ex);
					connection.release();
					res.sendStatus(500);
				}
			});
		} catch(ex) {
			console.log(ex);
			connection.release();
			res.sendStatus(500);
		}
	});
}

module.exports = {
	'CreateUpdate': CreateUpdate,
	'Retrieve': Retrieve,
	'Delete': Delete,
	'Search': Search,
}
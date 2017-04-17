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

function getLimit(req, columns) {
	var limit = '';
	
	var len = parseInt(req.body['length']);
	if(len > 100) {
		len = 100;
	}
	
	if(req.body.hasOwnProperty('start') && len != -1) {
		limit = 'LIMIT ' + req.body['start'] + ', ' + req.body['length'];
	}
	
	return limit;
}

function getOrder(req, columns) {
	var order = '';
	
	if (req.body.hasOwnProperty('order') && req.body['order'].length) {
		var orderBy = [];
		for(var i = 0; i < req.body['order'].length; ++i) {
			var columnIdx = parseInt(req.body['order'][i].column);
			var column = req.body['columns'][columnIdx];
			var columnId = column.data;
			var dir = req.body['order'][i].dir === 'asc' ? 'ASC' : 'DESC';			
			orderBy.push(req.mysql.escapeId(columnId) + ' ' + dir);
		}
		
		order = 'ORDER BY ' + orderBy.join(',');
	}
	
	return order;
}

function filter(req, columns, args) {
	var where = '';
	var globalSearch = [];
	var columnSearch = [];
	
	if (req.body.hasOwnProperty('search') && req.body['search'].value.length) {
		var searchTerm = req.body['search'].value;
		for(var i = 0; i < req.body['columns'].length; ++i) {
			var column = req.body['columns'][i];
			
			if(column.searchable) {
				globalSearch.push(req.mysql.escapeId(column.data) + ' LIKE ?');
				args.push('%' + searchTerm + '%');
			}
		}
	}
	
	for(var i = 0; i < req.body['columns'].length; ++i) {
		var column = req.body['columns'][i];
		var searchTerm = column.search.value;
		
		if(searchTerm !== '' && column.searchable) {
			columnSearch.push(req.mysql.escapeId(column.data) + ' LIKE ?');
			args.push('%' + searchTerm + '%');
		}
	}
	
	if(globalSearch.length) {
		where = globalSearch.join(' OR ');
	}
	
	if(columnSearch.length) {
		if(where === '') {
			where = columnSearch.join(' AND ');
		} else {
			where += ' AND ';
			where += columnSearch.join(' AND ');
		}
	}
	
	if(where !== '') {
		where = 'WHERE ' + where;
	}
	
	return where;
}

function Search(req, res, table, pkey, columns) {
	var args = [];
	var limit = getLimit(req, columns);
	var order = getOrder(req, columns);
	var where = filter(req, columns, args);

	var query = 'SELECT ' + columns.join(', ') + ' FROM ' + table + ' ' + where + ' ' + order + ' ' + limit;
	
	req.mysql.getConnection(function(err, connection) {
		try {
			if(err) {
				console.log(err);
				connection.release();
				res.sendStatus(500);
				return;
			}
			
			connection.query(query, args, function (error, results, fields) {
				try {
					var ret = [];
					
					for(var i in results) {
						var result = results[i];
						
						var obj = { };
						for(var idx in result) {
							var value = result[idx];
							obj[idx] = value;
						}
						
						ret.push(obj);
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
};

module.exports = {
	'CreateUpdate': CreateUpdate,
	'Retrieve': Retrieve,
	'Delete': Delete,
	'Search': Search,
}
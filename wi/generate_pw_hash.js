var sodium = require('libsodium-wrappers-sumo');

var hash = sodium.crypto_pwhash_str('password', 3, 32768);
console.log(hash);
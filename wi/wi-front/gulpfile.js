'use strict';

var del = require('del');
var fs = require('fs');
var browserify = require('browserify');
var gulp = require('gulp');
var source = require('vinyl-source-stream');
var buffer = require('vinyl-buffer');
var uglify = require('gulp-uglify');
var sourcemaps = require('gulp-sourcemaps');
var gutil = require('gulp-util');
var concat = require('gulp-concat');
var connect = require('gulp-connect');
var templateCache = require('gulp-angular-templatecache');
var minifyCSS = require('gulp-cssnano');
var minifyHTML = require('gulp-htmlmin');
var sass = require('gulp-sass');
var gulpsync = require('gulp-sync')(gulp);

gulp.task('clean:css', function() {
	return del([
		'dist/css/**/*'
	]);
});

gulp.task('clean:javascript', function() {
	return del([
		'dist/js/**/*'
	]);
});

gulp.task('clean', ['clean:css', 'clean:javascript']);

gulp.task('app:javascript:prod', function () {
  var b = browserify({
    entries: 'src/index.js',
    debug: false
  });

  return b.transform('babelify', {presets: ["es2015"]})
    .transform('brfs')
    .bundle()
    .pipe(source('bundle.js'))
    .pipe(buffer())
    .pipe(sourcemaps.init({loadMaps: true}))
        .pipe(uglify())
        .on('error', gutil.log)
    .pipe(sourcemaps.write('./'))
    .pipe(gulp.dest('./dist/js/'))
	.pipe(connect.reload());
});

gulp.task('app:javascript:dev', function () {
  var b = browserify({
    entries: 'src/index.js',
    debug: true
  });

  return b.transform('babelify', {presets: ["es2015"]})
    .transform('brfs')
    .bundle()
    .pipe(source('bundle.js'))
    .pipe(buffer())
    .pipe(sourcemaps.init({loadMaps: true}))
        .on('error', gutil.log)
    .pipe(sourcemaps.write('./'))
    .pipe(gulp.dest('./dist/js/'))
	.pipe(connect.reload());
});

gulp.task('javascript:dev', ['app:javascript:dev']);
gulp.task('javascript:prod', ['app:javascript:prod']);

var vendor_css = [
	'node_modules/angular-material/angular-material.min.css',
	'node_modules/angular-loading-bar/build/loading-bar.min.css',
	'node_modules/mdi/css/materialdesignicons.min.css'
];

gulp.task('vendor:css:prod', function() {
	return gulp.src(vendor_css)
		.pipe(minifyCSS())
		.pipe(concat('vendor.css'))
		.pipe(gulp.dest('dist/css'))
		.pipe(connect.reload());
});

gulp.task('vendor:css:dev', function() {
	return gulp.src(vendor_css)
		.pipe(concat('vendor.css'))
		.pipe(gulp.dest('dist/css'))
		.pipe(connect.reload());
});

var app_sass = [
	'src/**/*.scss',
];

gulp.task('app:sass:prod', function() {
	return gulp.src(app_sass)
		.pipe(sass().on('error', sass.logError))
		.pipe(minifyCSS())
		.pipe(concat('app.css'))
		.pipe(gulp.dest('dist/css'))
		.pipe(connect.reload());
});

gulp.task('app:sass:dev', function() {
	return gulp.src(app_sass)
		.pipe(sass().on('error', sass.logError))
		.pipe(concat('app.css'))
		.pipe(gulp.dest('dist/css'))
		.pipe(connect.reload());
});

gulp.task('css:prod', ['vendor:css:prod', 'app:sass:prod']);
gulp.task('css:dev', ['vendor:css:dev', 'app:sass:dev']);

var assets_images = [
	'src/assets/images/**/*',
];

var assets_media = [
	'src/assets/media/*',
];

gulp.task('assets:images', function() {
	return gulp.src(assets_images)
		.pipe(gulp.dest('dist/images'))
		.pipe(connect.reload());
});

gulp.task('assets:media', function() {
	return gulp.src(assets_media)
		.pipe(gulp.dest('dist/media'))
		.pipe(connect.reload());
});

var assets_fonts = [ 
	'node_modules/mdi/fonts/*'
];

gulp.task('assets:fonts', function() {
	return gulp.src(assets_fonts)
		.pipe(gulp.dest('dist/fonts'));
});

gulp.task('assets', ['assets:images', 'assets:media', 'assets:fonts']);

var app_html = [
	'src/**/*.html'
];

gulp.task('app:html:prod', function() {
	return gulp.src(app_html)
		.pipe(minifyHTML({collapseWhitespace: true, removeComments: true})) 
		.pipe(templateCache({standalone: true}))
		.pipe(gulp.dest('dist/js'))
		.pipe(connect.reload());
});

gulp.task('app:html:dev', function() {
	return gulp.src(app_html)
		.pipe(templateCache({standalone: true}))
		.pipe(gulp.dest('dist/js'))
		.pipe(connect.reload());
});

var app_entry = [
	'src/index.html'
];

gulp.task('app:entry:prod', function() {
	return gulp.src(app_entry)
		.pipe(minifyHTML({collapseWhitespace: true, removeComments: true})) 
		.pipe(gulp.dest('dist'))
		.pipe(connect.reload());
});

gulp.task('app:entry:dev', function() {
	return gulp.src(app_entry)
		.pipe(gulp.dest('dist'))
		.pipe(connect.reload());
});

gulp.task('build:prod', gulpsync.sync(['clean', 'javascript:prod', 'css:prod', 'app:html:prod', 'app:entry:prod', 'assets']));
gulp.task('build:dev', gulpsync.sync(['clean', 'javascript:dev', 'css:dev', 'app:html:dev', 'app:entry:dev', 'assets']));
gulp.task('build', ['build:prod']);

gulp.task('live_reload', function() {
	connect.server({
		livereload: true,
		root: 'dist',
		port: '8000'
	});
});

gulp.task('watch', ['live_reload'], function() {
	gulp.watch('src/**/*.js', ['app:javascript:dev']);
	gulp.watch(vendor_css, ['vendor:css:dev']);
	gulp.watch(app_sass, ['app:sass:dev']);
	gulp.watch(assets_images, ['assets:images']);
	gulp.watch(assets_media, ['assets:media']);
	gulp.watch(assets_fonts, ['assets:fonts']);
	gulp.watch(app_html, ['app:html:dev']);
	gulp.watch(app_entry, ['app:entry:dev']);
});

gulp.task('serve', gulpsync.sync(['build:dev', 'watch']));

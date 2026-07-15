const assert = require('node:assert/strict');
const fs = require('node:fs');
const path = require('node:path');
const test = require('node:test');

const { chooseGridStepMm, formatGridCoordinate, shouldShowDistanceLabel } = require('../nav_marker_grid_scale.js');

test('keeps one grid cell equal to one metre at every display zoom', () => {
  assert.equal(chooseGridStepMm(0.035), 1000);
  assert.equal(chooseGridStepMm(0.01), 1000);
  assert.equal(chooseGridStepMm(0.2), 1000);
});

test('formats five-grid coordinates in metres without noisy decimals', () => {
  assert.equal(formatGridCoordinate(5000), '5 m');
  assert.equal(formatGridCoordinate(-2500), '-2.5 m');
  assert.equal(formatGridCoordinate(0), '0 m');
});

test('only renders a five-grid distance label while the display switch is on', () => {
  assert.equal(typeof shouldShowDistanceLabel, 'function');
  if (typeof shouldShowDistanceLabel !== 'function') return;

  assert.equal(shouldShowDistanceLabel(true, 5), true);
  assert.equal(shouldShowDistanceLabel(true, 4), false);
  assert.equal(shouldShowDistanceLabel(false, 5), false);
});

test('loads the scale helper before the map renderer and keeps the inline script valid', () => {
  const htmlPath = path.join(__dirname, '..', 'nav_marker.html');
  const html = fs.readFileSync(htmlPath, 'utf8');
  const scripts = [...html.matchAll(/<script(?:[^>]*)?>([\s\S]*?)<\/script>/g)];
  const inlineMapScript = scripts.map((match) => match[1]).find((script) => script.includes('function grid'));

  assert.match(html, /<script src="nav_marker_grid_scale\.js"><\/script>/);
  assert.ok(inlineMapScript, 'map renderer script should exist');
  assert.doesNotThrow(() => new Function(inlineMapScript));
  assert.match(inlineMapScript, /shouldShowDistanceLabel\(S\.showRealDistance,xIndex\)/);
  assert.match(inlineMapScript, /shouldShowDistanceLabel\(S\.showRealDistance,yIndex\)/);
});

test('all navigation host variants use the same fixed vehicle-coordinate grid', () => {
  const toolRoot = path.join(__dirname, '..', '..');
  const hostDirs = [
    'webview_nav_marker',
    'webview_nav_marker速度规划',
    'webview_nav_marker速度规划_科目二',
  ];

  for (const hostDir of hostDirs) {
    const dir = path.join(toolRoot, hostDir);
    const html = fs.readFileSync(path.join(dir, 'nav_marker.html'), 'utf8');
    const helperPath = path.join(dir, 'nav_marker_grid_scale.js');
    const scripts = [...html.matchAll(/<script(?:[^>]*)?>([\s\S]*?)<\/script>/g)];
    const inlineMapScript = scripts.map((match) => match[1]).find((script) => script.includes('function grid'));

    assert.match(html, /<script src="nav_marker_grid_scale\.js"><\/script>/);
    assert.match(html, /const stepMm\s*=\s*1000/);
    assert.ok(inlineMapScript, `${hostDir} should retain its map renderer`);
    assert.doesNotThrow(() => new Function(inlineMapScript));
    assert.equal(fs.existsSync(helperPath), true, `${hostDir} should contain the shared scale helper`);
    const helper = fs.readFileSync(helperPath, 'utf8');
    assert.match(helper, /return 1000;/);
  }
});

test('all navigation host variants expose the real-distance display switch', () => {
  const toolRoot = path.join(__dirname, '..', '..');
  const hostDirs = [
    'webview_nav_marker',
    'webview_nav_marker速度规划',
    'webview_nav_marker速度规划_科目二',
  ];

  for (const hostDir of hostDirs) {
    const html = fs.readFileSync(path.join(toolRoot, hostDir, 'nav_marker.html'), 'utf8');

    assert.match(html, /<button id="bRealDistance" class="active">真实距离：开<\/button>/);
    assert.match(html, /showRealDistance:true/);
    assert.match(html, /NavMarkerGridScale\.shouldShowDistanceLabel\(S\.showRealDistance,xIndex\)/);
    assert.match(html, /NavMarkerGridScale\.shouldShowDistanceLabel\(S\.showRealDistance,yIndex\)/);
    assert.match(html, /E\.bRealDistance\.onclick=/);
  }
});

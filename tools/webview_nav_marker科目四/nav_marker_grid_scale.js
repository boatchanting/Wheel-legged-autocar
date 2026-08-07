(function (root, factory) {
  const api = factory();
  if (typeof module === 'object' && module.exports) module.exports = api;
  if (root) root.NavMarkerGridScale = api;
})(typeof window !== 'undefined' ? window : globalThis, function () {
  function chooseGridStepMm() {
    return 1000;
  }

  function formatGridCoordinate(mm) {
    const metres = mm / 1000;
    const text = metres.toFixed(3).replace(/\.0+$/, '').replace(/(\.\d*?)0+$/, '$1');
    return `${text} m`;
  }

  function shouldShowDistanceLabel(showDistance, gridIndex) {
    return Boolean(showDistance) && Number.isInteger(gridIndex) && gridIndex % 5 === 0;
  }

  return { chooseGridStepMm, formatGridCoordinate, shouldShowDistanceLabel };
});

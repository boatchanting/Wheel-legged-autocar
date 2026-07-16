const LINE_TYPES = {
  inner: { label: "内框线", color: [0, 220, 0] },
  outer: { label: "外框线", color: [255, 0, 0] },
};

const TYPE_ORDER = ["inner", "outer"];
const MIN_ZOOM = 2;
const MAX_ZOOM = 20;
const SESSION_STORAGE_KEY = "minefield-html-labeler-session";
const HANDLE_DB_NAME = "minefield-html-labeler-db";
const HANDLE_STORE_NAME = "handles";
const EMPTY_VERSION = JSON.stringify({
  clearedTypes: [],
  segments: { inner: [], outer: [] },
});

const elements = {
  canvas: document.getElementById("annotator-canvas"),
  canvasScroll: document.getElementById("canvas-scroll"),
  fileList: document.getElementById("file-list"),
  fileCount: document.getElementById("file-count"),
  currentFile: document.getElementById("current-file"),
  frameStatus: document.getElementById("frame-status"),
  activeToolLabel: document.getElementById("active-tool-label"),
  pointerLabel: document.getElementById("pointer-label"),
  zoomLabel: document.getElementById("zoom-label"),
  drawDirLabel: document.getElementById("draw-dir-label"),
  originalDirLabel: document.getElementById("original-dir-label"),
  outputDirLabel: document.getElementById("output-dir-label"),
  replaceOnDraw: document.getElementById("replace-on-draw"),
  drawInput: document.getElementById("draw-input"),
  originalInput: document.getElementById("original-input"),
  acceptAlgorithm: document.getElementById("accept-algorithm"),
};

const state = {
  zoom: 6,
  activeType: "inner",
  pendingStart: null,
  previewPoint: null,
  currentIndex: -1,
  entries: [],
  entryState: new Map(),
  drawSource: null,
  originalSource: null,
  outputDirHandle: null,
  currentFrame: null,
  busy: false,
  suspendSessionPersist: false,
};

const ctx = elements.canvas.getContext("2d", { willReadFrequently: true });

function openHandleDb() {
  return new Promise((resolve, reject) => {
    const request = indexedDB.open(HANDLE_DB_NAME, 1);
    request.onupgradeneeded = () => {
      const db = request.result;
      if (!db.objectStoreNames.contains(HANDLE_STORE_NAME)) {
        db.createObjectStore(HANDLE_STORE_NAME);
      }
    };
    request.onsuccess = () => resolve(request.result);
    request.onerror = () => reject(request.error);
  });
}

async function setStoredHandle(key, handle) {
  const db = await openHandleDb();
  await new Promise((resolve, reject) => {
    const tx = db.transaction(HANDLE_STORE_NAME, "readwrite");
    tx.objectStore(HANDLE_STORE_NAME).put(handle, key);
    tx.oncomplete = () => resolve();
    tx.onerror = () => reject(tx.error);
  });
  db.close();
}

async function getStoredHandle(key) {
  const db = await openHandleDb();
  const handle = await new Promise((resolve, reject) => {
    const tx = db.transaction(HANDLE_STORE_NAME, "readonly");
    const request = tx.objectStore(HANDLE_STORE_NAME).get(key);
    request.onsuccess = () => resolve(request.result || null);
    request.onerror = () => reject(request.error);
  });
  db.close();
  return handle;
}

function persistSessionState() {
  if (state.suspendSessionPersist) {
    return;
  }
  const payload = {
    currentIndex: state.currentIndex,
    activeType: state.activeType,
    zoom: state.zoom,
    replaceOnDraw: elements.replaceOnDraw.checked,
    drawDirLabel: elements.drawDirLabel.textContent,
    originalDirLabel: elements.originalDirLabel.textContent,
    outputDirLabel: elements.outputDirLabel.textContent,
  };
  localStorage.setItem(SESSION_STORAGE_KEY, JSON.stringify(payload));
}

function loadSessionState() {
  try {
    const raw = localStorage.getItem(SESSION_STORAGE_KEY);
    return raw ? JSON.parse(raw) : null;
  } catch (error) {
    console.error(error);
    return null;
  }
}

async function tryRestoreDirectoryHandle(key) {
  if (!window.showDirectoryPicker) {
    return null;
  }
  try {
    return await getStoredHandle(key);
  } catch (error) {
    console.error(error);
    return null;
  }
}

function normalizeKey(name) {
  const lower = name.toLowerCase();
  if (lower.includes("__")) {
    return lower.split("__").pop();
  }
  return lower;
}

function naturalCompare(a, b) {
  return a.localeCompare(b, "zh-CN", { numeric: true, sensitivity: "base" });
}

function setStatus(text) {
  elements.frameStatus.textContent = text;
}

function setPointerHint(text) {
  elements.pointerLabel.textContent = text;
}

function clamp(value, min, max) {
  return Math.max(min, Math.min(max, value));
}

function cloneSegment(segment) {
  return {
    x1: segment.x1,
    y1: segment.y1,
    x2: segment.x2,
    y2: segment.y2,
  };
}

function createEmptyEditState() {
  return {
    clearedTypes: new Set(),
    segments: {
      inner: [],
      outer: [],
    },
    savedVersion: null,
    history: [],
    historyIndex: -1,
  };
}

function snapshotEditState(editState) {
  return {
    clearedTypes: Array.from(editState.clearedTypes),
    segments: TYPE_ORDER.reduce((acc, type) => {
      acc[type] = editState.segments[type].map(cloneSegment);
      return acc;
    }, {}),
  };
}

function restoreSnapshot(editState, snapshot) {
  editState.clearedTypes = new Set(snapshot.clearedTypes);
  editState.segments = TYPE_ORDER.reduce((acc, type) => {
    acc[type] = snapshot.segments[type].map(cloneSegment);
    return acc;
  }, {});
}

function buildVersionKey(editState) {
  return JSON.stringify(snapshotEditState(editState));
}

function hasRealEdits(editState) {
  return buildVersionKey(editState) !== EMPTY_VERSION;
}

function ensureHistory(editState) {
  if (editState.history.length === 0) {
    const initial = snapshotEditState(editState);
    editState.history.push(initial);
    editState.historyIndex = 0;
  }
}

function pushHistory(editState) {
  ensureHistory(editState);
  const current = snapshotEditState(editState);
  const latest = editState.history[editState.historyIndex];
  if (JSON.stringify(latest) === JSON.stringify(current)) {
    return;
  }
  editState.history = editState.history.slice(0, editState.historyIndex + 1);
  editState.history.push(current);
  editState.historyIndex += 1;
}

function getCurrentEntry() {
  return state.entries[state.currentIndex] || null;
}

function getCurrentEditState() {
  const entry = getCurrentEntry();
  if (!entry) {
    return null;
  }
  if (!state.entryState.has(entry.id)) {
    const editState = createEmptyEditState();
    ensureHistory(editState);
    state.entryState.set(entry.id, editState);
  }
  return state.entryState.get(entry.id);
}

async function collectHandlesFromDirectory(dirHandle, prefix = "") {
  const results = [];
  for await (const [name, handle] of dirHandle.entries()) {
    if (handle.kind === "file" && name.toLowerCase().endsWith(".png")) {
      results.push({
        id: `${prefix}${name}`,
        name,
        relativePath: `${prefix}${name}`,
        handle,
      });
    } else if (handle.kind === "directory") {
      const nested = await collectHandlesFromDirectory(handle, `${prefix}${name}/`);
      results.push(...nested);
    }
  }
  return results.sort((a, b) => naturalCompare(a.relativePath, b.relativePath));
}

function collectHandlesFromInput(fileList) {
  const items = Array.from(fileList)
    .filter((file) => file.name.toLowerCase().endsWith(".png"))
    .map((file) => ({
      id: file.webkitRelativePath || file.name,
      name: file.name,
      relativePath: file.webkitRelativePath || file.name,
      file,
    }));
  items.sort((a, b) => naturalCompare(a.relativePath, b.relativePath));
  return items;
}

function mapEntries(entries) {
  const exact = new Map();
  const normalized = new Map();
  for (const entry of entries) {
    exact.set(entry.name.toLowerCase(), entry);
    normalized.set(normalizeKey(entry.name), entry);
  }
  return { exact, normalized };
}

function findMatchingEntry(entry, maps) {
  if (!entry || !maps) {
    return null;
  }
  const exactMatch = maps.exact.get(entry.name.toLowerCase());
  const normalizedMatch = maps.normalized.get(normalizeKey(entry.name));
  return exactMatch || normalizedMatch || null;
}

function rebuildEntries() {
  const originalEntries = state.originalSource?.entries || [];
  const drawEntries = state.drawSource?.entries || [];
  const originalMap = mapEntries(originalEntries);
  const drawMap = mapEntries(drawEntries);

  let baseEntries;
  if (drawEntries.length > 0) {
    baseEntries = drawEntries.map((entry) => ({
      ...entry,
      drawEntry: entry,
      originalEntry: findMatchingEntry(entry, originalMap),
    }));
  } else {
    baseEntries = originalEntries.map((entry) => ({
      ...entry,
      drawEntry: null,
      originalEntry: entry,
    }));
  }

  state.entries = baseEntries.sort((a, b) => naturalCompare(a.relativePath, b.relativePath));
}

async function getFileBlob(entry) {
  if (entry.handle) {
    return entry.handle.getFile();
  }
  return entry.file;
}

async function createImageDataFromEntry(entry) {
  const blob = await getFileBlob(entry);
  const bitmap = await createImageBitmap(blob);
  const bufferCanvas = document.createElement("canvas");
  bufferCanvas.width = bitmap.width;
  bufferCanvas.height = bitmap.height;
  const bufferCtx = bufferCanvas.getContext("2d", { willReadFrequently: true });
  bufferCtx.drawImage(bitmap, 0, 0);
  const imageData = bufferCtx.getImageData(0, 0, bitmap.width, bitmap.height);
  bitmap.close();
  return imageData;
}

function copyImageData(imageData) {
  return new ImageData(new Uint8ClampedArray(imageData.data), imageData.width, imageData.height);
}

function matchesColor(data, index, color) {
  return data[index] === color[0] && data[index + 1] === color[1] && data[index + 2] === color[2] && data[index + 3] === 255;
}

function clearColorPixels(baseImage, originalImage, color) {
  const work = copyImageData(baseImage);
  for (let index = 0; index < work.data.length; index += 4) {
    if (!matchesColor(work.data, index, color)) {
      continue;
    }
    if (originalImage) {
      work.data[index] = originalImage.data[index];
      work.data[index + 1] = originalImage.data[index + 1];
      work.data[index + 2] = originalImage.data[index + 2];
      work.data[index + 3] = originalImage.data[index + 3];
    }
  }
  return work;
}

function setPixel(imageData, x, y, color) {
  if (x < 0 || y < 0 || x >= imageData.width || y >= imageData.height) {
    return;
  }
  const index = (y * imageData.width + x) * 4;
  imageData.data[index] = color[0];
  imageData.data[index + 1] = color[1];
  imageData.data[index + 2] = color[2];
  imageData.data[index + 3] = 255;
}

function drawLineOnImageData(imageData, segment, color) {
  let x0 = segment.x1;
  let y0 = segment.y1;
  const x1 = segment.x2;
  const y1 = segment.y2;
  const dx = Math.abs(x1 - x0);
  const dy = Math.abs(y1 - y0);
  const sx = x0 < x1 ? 1 : -1;
  const sy = y0 < y1 ? 1 : -1;
  let err = dx - dy;

  while (true) {
    setPixel(imageData, x0, y0, color);
    if (x0 === x1 && y0 === y1) {
      break;
    }
    const twiceError = err * 2;
    if (twiceError > -dy) {
      err -= dy;
      x0 += sx;
    }
    if (twiceError < dx) {
      err += dx;
      y0 += sy;
    }
  }
}

function composeFrameImage() {
  const frame = state.currentFrame;
  const editState = getCurrentEditState();
  if (!frame || !editState) {
    return null;
  }

  let work = copyImageData(frame.baseImageData);
  for (const type of editState.clearedTypes) {
    work = clearColorPixels(work, frame.originalImageData, LINE_TYPES[type].color);
  }
  for (const type of TYPE_ORDER) {
    for (const segment of editState.segments[type]) {
      drawLineOnImageData(work, segment, LINE_TYPES[type].color);
    }
  }
  return work;
}

function updateCanvasScale() {
  const width = elements.canvas.width * state.zoom;
  const height = elements.canvas.height * state.zoom;
  elements.canvas.style.width = `${width}px`;
  elements.canvas.style.height = `${height}px`;
  elements.zoomLabel.textContent = `${state.zoom}x`;
}

function drawPreviewLine() {
  if (!state.pendingStart || !state.previewPoint) {
    return;
  }
  const color = LINE_TYPES[state.activeType].color;
  ctx.save();
  ctx.strokeStyle = `rgb(${color[0]}, ${color[1]}, ${color[2]})`;
  ctx.lineWidth = 1;
  ctx.beginPath();
  ctx.moveTo(state.pendingStart.x + 0.5, state.pendingStart.y + 0.5);
  ctx.lineTo(state.previewPoint.x + 0.5, state.previewPoint.y + 0.5);
  ctx.stroke();
  ctx.restore();
}

function refreshAcceptAlgorithmButton() {
  elements.acceptAlgorithm.disabled = !state.currentFrame?.algorithmEntry;
}

function renderCurrentFrame() {
  const composed = composeFrameImage();
  if (!composed) {
    ctx.clearRect(0, 0, elements.canvas.width, elements.canvas.height);
    refreshAcceptAlgorithmButton();
    return;
  }
  ctx.putImageData(composed, 0, 0);
  drawPreviewLine();
  renderFileList();
  refreshStatus();
  refreshAcceptAlgorithmButton();
  persistSessionState();
}

function refreshToolButtons() {
  for (const type of TYPE_ORDER) {
    const button = document.getElementById(`tool-${type}`);
    button.classList.toggle("active", type === state.activeType);
  }
  elements.activeToolLabel.textContent = LINE_TYPES[state.activeType].label;
  persistSessionState();
}

function refreshStatus() {
  const entry = getCurrentEntry();
  const editState = getCurrentEditState();
  elements.currentFile.textContent = entry ? entry.name : "未加载";
  if (!entry || !editState) {
    setStatus("等待目录");
    return;
  }
  const currentVersion = buildVersionKey(editState);
  if (editState.savedVersion && editState.savedVersion === currentVersion) {
    setStatus("已保存");
  } else if (hasRealEdits(editState)) {
    setStatus("已修改，未保存");
  } else {
    setStatus("未修改");
  }
}

function resetPendingLine() {
  state.pendingStart = null;
  state.previewPoint = null;
  setPointerHint("点击起点，再点击终点");
  renderCurrentFrame();
}

function setActiveType(type) {
  state.activeType = type;
  refreshToolButtons();
  setPointerHint(`当前绘制 ${LINE_TYPES[type].label}，点击起点，再点击终点`);
}

function clearType(type) {
  const editState = getCurrentEditState();
  if (!editState) {
    return;
  }
  if (!state.currentFrame?.originalImageData) {
    setStatus("当前未匹配原图，清空时无法恢复背景");
  }
  editState.clearedTypes.add(type);
  editState.segments[type] = [];
  pushHistory(editState);
  setActiveType(type);
  resetPendingLine();
}

function resetCurrentFrameEdits() {
  const editState = getCurrentEditState();
  if (!editState) {
    return;
  }
  editState.clearedTypes = new Set();
  for (const type of TYPE_ORDER) {
    editState.segments[type] = [];
  }
  pushHistory(editState);
  resetPendingLine();
}

function undoEdit() {
  const editState = getCurrentEditState();
  if (!editState || editState.historyIndex <= 0) {
    return;
  }
  editState.historyIndex -= 1;
  restoreSnapshot(editState, editState.history[editState.historyIndex]);
  resetPendingLine();
}

function redoEdit() {
  const editState = getCurrentEditState();
  if (!editState || editState.historyIndex >= editState.history.length - 1) {
    return;
  }
  editState.historyIndex += 1;
  restoreSnapshot(editState, editState.history[editState.historyIndex]);
  resetPendingLine();
}

async function writeBlobToOutput(name, blob) {
  if (!state.outputDirHandle) {
    throw new Error("NO_OUTPUT_DIR");
  }
  const fileHandle = await state.outputDirHandle.getFileHandle(name, { create: true });
  const writable = await fileHandle.createWritable();
  await writable.write(blob);
  await writable.close();
}

async function saveCurrentFrame(options = {}) {
  if (state.busy) {
    return false;
  }
  const entry = getCurrentEntry();
  const editState = getCurrentEditState();
  if (!entry || !editState) {
    return false;
  }
  if (!state.outputDirHandle) {
    setStatus("请先选择输出目录");
    return false;
  }

  state.busy = true;
  try {
    if (options.acceptAlgorithm) {
      if (!entry.drawEntry) {
        setStatus("当前没有算法图可直接保存");
        return false;
      }
      const blob = await getFileBlob(entry.drawEntry);
      await writeBlobToOutput(entry.name, blob);
      editState.clearedTypes = new Set();
      for (const type of TYPE_ORDER) {
        editState.segments[type] = [];
      }
      ensureHistory(editState);
      editState.savedVersion = buildVersionKey(editState);
      renderCurrentFrame();
      return true;
    }

    const composed = composeFrameImage();
    if (!composed) {
      return false;
    }

    const exportCanvas = document.createElement("canvas");
    exportCanvas.width = composed.width;
    exportCanvas.height = composed.height;
    exportCanvas.getContext("2d").putImageData(composed, 0, 0);
    const blob = await new Promise((resolve) => exportCanvas.toBlob(resolve, "image/png"));
    await writeBlobToOutput(entry.name, blob);
    editState.savedVersion = buildVersionKey(editState);
    renderCurrentFrame();
    return true;
  } catch (error) {
    if (error?.message === "NO_OUTPUT_DIR") {
      setStatus("请先选择输出目录");
    } else {
      console.error(error);
      setStatus("保存失败，请查看控制台");
    }
    return false;
  } finally {
    state.busy = false;
  }
}

function renderFileList() {
  const activeEntry = getCurrentEntry();
  elements.fileList.innerHTML = "";
  for (let index = 0; index < state.entries.length; index += 1) {
    const entry = state.entries[index];
    const editState = state.entryState.get(entry.id);
    const item = document.createElement("button");
    item.type = "button";
    item.className = "file-item";
    if (activeEntry && activeEntry.id === entry.id) {
      item.classList.add("active");
    }
    if (editState) {
      const version = buildVersionKey(editState);
      if (editState.savedVersion && editState.savedVersion === version) {
        item.classList.add("saved");
      } else if (hasRealEdits(editState)) {
        item.classList.add("modified");
      }
    }
    const baseLabel = entry.drawEntry ? "算法图" : "原图";
    const originalLabel = entry.originalEntry ? "已匹配原图" : "未匹配原图";
    item.innerHTML = `<span class="file-name">${entry.name}</span><span class="file-meta">${baseLabel} / ${originalLabel}</span>`;
    item.addEventListener("click", () => loadFrame(index));
    elements.fileList.appendChild(item);
  }
}

async function loadFrame(index) {
  if (index < 0 || index >= state.entries.length) {
    return;
  }
  state.currentIndex = index;
  const entry = state.entries[index];
  const baseEntry = entry.drawEntry || entry.originalEntry;
  if (!baseEntry) {
    setStatus("当前文件没有可加载的图像");
    return;
  }

  const baseImageData = await createImageDataFromEntry(baseEntry);
  let originalImageData = null;
  if (entry.originalEntry) {
    originalImageData = await createImageDataFromEntry(entry.originalEntry);
    if (originalImageData.width !== baseImageData.width || originalImageData.height !== baseImageData.height) {
      originalImageData = null;
      setStatus("原图尺寸不匹配，已忽略原图");
    }
  }
  state.currentFrame = {
    baseImageData,
    originalImageData,
    algorithmEntry: entry.drawEntry,
  };
  elements.canvas.width = baseImageData.width;
  elements.canvas.height = baseImageData.height;
  updateCanvasScale();
  resetPendingLine();
  renderCurrentFrame();
  persistSessionState();
}

async function rebuildAndRenderEntries() {
  rebuildEntries();
  state.currentFrame = null;
  if (!(state.entryState instanceof Map)) {
    state.entryState = new Map();
  }
  if (state.currentIndex < 0 || state.currentIndex >= state.entries.length) {
    state.currentIndex = -1;
  }
  elements.fileCount.textContent = `${state.entries.length} 张`;
  renderFileList();
  if (state.entries.length > 0) {
    await loadFrame(state.currentIndex >= 0 ? state.currentIndex : 0);
  } else {
    refreshStatus();
    refreshAcceptAlgorithmButton();
  }
  persistSessionState();
}

async function pickDrawDirectory() {
  if (!window.showDirectoryPicker) {
    elements.drawInput.click();
    return;
  }
  const dirHandle = await window.showDirectoryPicker({ mode: "read" });
  const entries = await collectHandlesFromDirectory(dirHandle);
  state.drawSource = { mode: "handle", entries, dirHandle };
  elements.drawDirLabel.textContent = dirHandle.name;
  await setStoredHandle("drawDirHandle", dirHandle);
  await rebuildAndRenderEntries();
}

async function pickOriginalDirectory() {
  if (!window.showDirectoryPicker) {
    elements.originalInput.click();
    return;
  }
  const dirHandle = await window.showDirectoryPicker({ mode: "read" });
  const entries = await collectHandlesFromDirectory(dirHandle);
  state.originalSource = { mode: "handle", entries, dirHandle };
  elements.originalDirLabel.textContent = dirHandle.name;
  await setStoredHandle("originalDirHandle", dirHandle);
  await rebuildAndRenderEntries();
}

async function pickOutputDirectory() {
  if (!window.showDirectoryPicker) {
    window.alert("当前浏览器不支持目录写入，将改为下载 PNG。建议用 Edge 或 Chrome 打开 localhost。");
    return;
  }
  const dirHandle = await window.showDirectoryPicker({ mode: "readwrite" });
  state.outputDirHandle = dirHandle;
  elements.outputDirLabel.textContent = dirHandle.name;
  await setStoredHandle("outputDirHandle", dirHandle);
  persistSessionState();
}

async function loadDrawInputFiles() {
  const entries = collectHandlesFromInput(elements.drawInput.files);
  state.drawSource = { mode: "input", entries };
  elements.drawDirLabel.textContent = "已从文件夹导入";
  persistSessionState();
  await rebuildAndRenderEntries();
}

async function loadOriginalInputFiles() {
  const entries = collectHandlesFromInput(elements.originalInput.files);
  state.originalSource = { mode: "input", entries };
  elements.originalDirLabel.textContent = "已从文件夹导入";
  persistSessionState();
  await rebuildAndRenderEntries();
}

function updateZoom(step) {
  state.zoom = clamp(state.zoom + step, MIN_ZOOM, MAX_ZOOM);
  updateCanvasScale();
  persistSessionState();
}

function getCanvasPoint(event) {
  const rect = elements.canvas.getBoundingClientRect();
  const scaleX = elements.canvas.width / rect.width;
  const scaleY = elements.canvas.height / rect.height;
  const x = clamp(Math.round((event.clientX - rect.left) * scaleX), 0, elements.canvas.width - 1);
  const y = clamp(Math.round((event.clientY - rect.top) * scaleY), 0, elements.canvas.height - 1);
  return { x, y };
}

function addSegmentAtPoint(point) {
  const editState = getCurrentEditState();
  if (!editState) {
    return;
  }
  if (!state.pendingStart) {
    state.pendingStart = point;
    state.previewPoint = point;
    setPointerHint(`已记录起点 (${point.x}, ${point.y})，点击终点完成`);
    renderCurrentFrame();
    return;
  }

  if (elements.replaceOnDraw.checked) {
    editState.clearedTypes.add(state.activeType);
    editState.segments[state.activeType] = [];
  }

  editState.segments[state.activeType].push({
    x1: state.pendingStart.x,
    y1: state.pendingStart.y,
    x2: point.x,
    y2: point.y,
  });
  pushHistory(editState);
  resetPendingLine();
}

async function goToSiblingFrame(delta) {
  if (state.entries.length === 0) {
    return;
  }
  const nextIndex = clamp(state.currentIndex + delta, 0, state.entries.length - 1);
  if (nextIndex !== state.currentIndex) {
    await loadFrame(nextIndex);
  }
}

async function saveAndMove(delta, options = {}) {
  const saved = await saveCurrentFrame(options);
  if (saved) {
    await goToSiblingFrame(delta);
  }
}

function handleCanvasMove(event) {
  if (!state.pendingStart) {
    return;
  }
  state.previewPoint = getCanvasPoint(event);
  renderCurrentFrame();
}

function handleCanvasLeave() {
  state.previewPoint = null;
  renderCurrentFrame();
}

function bindEvents() {
  document.getElementById("pick-draw-dir").addEventListener("click", pickDrawDirectory);
  document.getElementById("pick-original-dir").addEventListener("click", pickOriginalDirectory);
  document.getElementById("pick-output-dir").addEventListener("click", pickOutputDirectory);
  document.getElementById("fallback-draw-dir").addEventListener("click", () => elements.drawInput.click());
  document.getElementById("fallback-original-dir").addEventListener("click", () => elements.originalInput.click());

  elements.drawInput.addEventListener("change", loadDrawInputFiles);
  elements.originalInput.addEventListener("change", loadOriginalInputFiles);

  document.getElementById("prev-frame").addEventListener("click", () => goToSiblingFrame(-1));
  document.getElementById("next-frame").addEventListener("click", () => goToSiblingFrame(1));
  document.getElementById("save-frame").addEventListener("click", () => saveCurrentFrame());
  document.getElementById("save-next-frame").addEventListener("click", () => saveAndMove(1));
  document.getElementById("accept-algorithm").addEventListener("click", () => saveCurrentFrame({ acceptAlgorithm: true }));
  document.getElementById("reset-frame").addEventListener("click", resetCurrentFrameEdits);
  document.getElementById("undo-action").addEventListener("click", undoEdit);
  document.getElementById("redo-action").addEventListener("click", redoEdit);
  document.getElementById("clear-inner").addEventListener("click", () => clearType("inner"));
  document.getElementById("clear-outer").addEventListener("click", () => clearType("outer"));
  document.getElementById("zoom-in").addEventListener("click", () => updateZoom(1));
  document.getElementById("zoom-out").addEventListener("click", () => updateZoom(-1));
  document.getElementById("fit-zoom").addEventListener("click", () => {
    state.zoom = 6;
    updateCanvasScale();
    persistSessionState();
  });

  elements.replaceOnDraw.addEventListener("change", () => {
    persistSessionState();
  });

  for (const type of TYPE_ORDER) {
    document.getElementById(`tool-${type}`).addEventListener("click", () => setActiveType(type));
  }

  elements.canvas.addEventListener("click", (event) => addSegmentAtPoint(getCanvasPoint(event)));
  elements.canvas.addEventListener("mousemove", handleCanvasMove);
  elements.canvas.addEventListener("mouseleave", handleCanvasLeave);

  document.addEventListener("keydown", async (event) => {
    if (event.target instanceof HTMLInputElement || event.target instanceof HTMLTextAreaElement) {
      return;
    }
    if (event.ctrlKey && event.key.toLowerCase() === "s") {
      event.preventDefault();
      if (event.shiftKey) {
        await saveAndMove(1);
      } else {
        await saveCurrentFrame();
      }
      return;
    }
    if (event.ctrlKey && event.key.toLowerCase() === "z") {
      event.preventDefault();
      undoEdit();
      return;
    }
    if (event.ctrlKey && event.key.toLowerCase() === "y") {
      event.preventDefault();
      redoEdit();
      return;
    }

    switch (event.key) {
      case "1":
        setActiveType("inner");
        break;
      case "2":
        setActiveType("outer");
        break;
      case "ArrowLeft":
        event.preventDefault();
        clearType("inner");
        break;
      case "ArrowRight":
        event.preventDefault();
        clearType("outer");
        break;
      case "n":
      case "N":
        event.preventDefault();
        await goToSiblingFrame(-1);
        break;
      case "m":
      case "M":
        event.preventDefault();
        await goToSiblingFrame(1);
        break;
      case "a":
      case "A":
        event.preventDefault();
        await saveCurrentFrame({ acceptAlgorithm: true });
        break;
      case "S":
        event.preventDefault();
        await saveAndMove(1);
        break;
      case "[":
        updateZoom(-1);
        break;
      case "]":
        updateZoom(1);
        break;
      case "Escape":
        resetPendingLine();
        break;
      case "Backspace":
        event.preventDefault();
        resetCurrentFrameEdits();
        break;
      default:
        break;
    }
  });
}

async function restoreSession() {
  state.suspendSessionPersist = true;
  try {
    const saved = loadSessionState();
    if (saved) {
      state.zoom = clamp(saved.zoom || 6, MIN_ZOOM, MAX_ZOOM);
      state.currentIndex = Number.isInteger(saved.currentIndex) ? saved.currentIndex : -1;
      elements.replaceOnDraw.checked = saved.replaceOnDraw === true;
      elements.drawDirLabel.textContent = saved.drawDirLabel || "未选择（可留空）";
      elements.originalDirLabel.textContent = saved.originalDirLabel || "未选择";
      elements.outputDirLabel.textContent = saved.outputDirLabel || "未选择";
      if (saved.activeType && LINE_TYPES[saved.activeType]) {
        state.activeType = saved.activeType;
      }
    }

    const originalHandle = await tryRestoreDirectoryHandle("originalDirHandle");
    if (originalHandle) {
      try {
        const entries = await collectHandlesFromDirectory(originalHandle);
        state.originalSource = { mode: "handle", entries, dirHandle: originalHandle };
        elements.originalDirLabel.textContent = originalHandle.name;
      } catch (error) {
        console.error(error);
      }
    }

    const outputHandle = await tryRestoreDirectoryHandle("outputDirHandle");
    if (outputHandle) {
      state.outputDirHandle = outputHandle;
      elements.outputDirLabel.textContent = outputHandle.name;
    }

    const drawHandle = await tryRestoreDirectoryHandle("drawDirHandle");
    if (drawHandle) {
      try {
        const entries = await collectHandlesFromDirectory(drawHandle);
        state.drawSource = { mode: "handle", entries, dirHandle: drawHandle };
        elements.drawDirLabel.textContent = drawHandle.name;
      } catch (error) {
        console.error(error);
      }
    }

    if (state.originalSource || state.drawSource) {
      await rebuildAndRenderEntries();
      setStatus("已恢复上次会话");
    }
  } finally {
    state.suspendSessionPersist = false;
    persistSessionState();
  }
}

async function initialize() {
  state.suspendSessionPersist = true;
  try {
    bindEvents();
    setActiveType("inner");
    updateCanvasScale();
    refreshStatus();
    refreshAcceptAlgorithmButton();
    await restoreSession();
    setActiveType(state.activeType);
    updateCanvasScale();
  } finally {
    state.suspendSessionPersist = false;
    persistSessionState();
  }
}

initialize();

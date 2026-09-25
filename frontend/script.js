const colors = { W: ['White', 'color-W'], Y: ['Yellow', 'color-Y'], G: ['Green', 'color-G'], B: ['Blue', 'color-B'], O: ['Orange', 'color-O'], R: ['Red', 'color-R'] };
const faces = ['U', 'D', 'F', 'B', 'L', 'R'];
let state = [...'W'.repeat(9) + 'Y'.repeat(9) + 'G'.repeat(9) + 'B'.repeat(9) + 'O'.repeat(9) + 'R'.repeat(9)];
let selectedColor = 'W'; let solutionMoves = []; let progress = 0; let timer = null; let solutionStart = [...state];
const palette = document.querySelector('#palette'); const net = document.querySelector('#cube-net');
const faceLocations = { U: 0, D: 9, F: 18, B: 27, L: 36, R: 45 };
const apiBase = window.location.port === '3000' ? '' : 'http://localhost:3000';

function renderPalette() {
  palette.innerHTML = Object.entries(colors).map(([key, value]) => `<button class="swatch ${value[1]} ${key === selectedColor ? 'selected' : ''}" data-color="${key}" title="Select ${value[0]}"><small>${value[0]}</small></button>`).join('');
  palette.querySelectorAll('.swatch').forEach(button => button.onclick = () => { selectedColor = button.dataset.color; renderPalette(); });
}
function renderCube() {
  net.innerHTML = faces.map(face => `<div class="face ${face}">${Array.from({ length: 9 }, (_, index) => { const absolute = faceLocations[face] + index; const center = index === 4 ? ' center' : ''; return `<button class="sticker ${colors[state[absolute]][1]}${center}" data-index="${absolute}" title="${face}${index + 1}"></button>`; }).join('')}</div>`).join('');
  net.querySelectorAll('.sticker:not(.center)').forEach(sticker => sticker.onclick = () => { state[Number(sticker.dataset.index)] = selectedColor; renderCube(); updateStateLabel(); });
  updateStateLabel();
}
function updateStateLabel() { document.querySelector('#state-count').textContent = state.join('') === solvedState() ? 'Solved' : 'Edited state'; }
function solvedState() { return 'W'.repeat(9) + 'Y'.repeat(9) + 'G'.repeat(9) + 'B'.repeat(9) + 'O'.repeat(9) + 'R'.repeat(9); }
function setMessage(text, good = false) { const element = document.querySelector('#validation'); element.textContent = text; element.className = `message ${good ? 'good' : 'bad'}`; }
async function post(path) {
  let response;
  try {
    response = await fetch(`${apiBase}${path}`, { method: 'POST', headers: { 'Content-Type': 'application/json' }, body: JSON.stringify({ cube: state.join('') }) });
  } catch (_error) {
    throw new Error('Cannot connect to the backend. Run npm.cmd start and open http://localhost:3000.');
  }
  const body = await response.text();
  let result;
  try { result = body ? JSON.parse(body) : null; } catch (_error) { throw new Error(`Server returned invalid JSON (${response.status}).`); }
  if (!result) throw new Error(`Server returned an empty response (${response.status}). Is the backend running on port 3000?`);
  if (!response.ok) throw new Error(result.error || 'Request failed');
  return result;
}

document.querySelector('#validate').onclick = async () => { setMessage('Checking cubie orientation and permutation...'); try { const result = await post('/api/validate'); setMessage(result.message || result.error, result.solvable); } catch (error) { setMessage(error.message); } };
document.querySelector('#solve').onclick = async () => { setMessage('C++ bidirectional BFS is searching...'); document.querySelector('#solve').disabled = true; try { const result = await post('/api/solve'); if (!result.solvable) { setMessage(result.error || 'No solution found.'); return; } solutionStart = [...state]; solutionMoves = result.moves; progress = 0; document.querySelector('#solution').textContent = solutionMoves.join(' ') || 'Already solved'; document.querySelector('#move-count').textContent = result.moveCount; document.querySelector('#benchmark').innerHTML = `<strong>Search telemetry</strong><br>States: ${result.statesExplored.toLocaleString()}<br>Queue peak: ${result.maxQueueSize.toLocaleString()}<br>Time: ${Number(result.milliseconds).toFixed(1)} ms`; setMessage('Valid cube. Solution ready to play.', true); updateProgress(); } catch (error) { setMessage(error.message); } finally { document.querySelector('#solve').disabled = false; } };
document.querySelector('#reset').onclick = () => { stop(); state = [...solvedState()]; solutionStart = [...state]; solutionMoves = []; progress = 0; renderCube(); document.querySelector('#solution').textContent = 'Awaiting validation'; document.querySelector('#move-count').textContent = '--'; updateProgress(); setMessage('Reset to solved cube.', true); };
document.querySelector('#scramble').onclick = () => { stop(); state = [...solvedState()]; const moves = []; let previous = ''; for (let i = 0; i < 8; i++) { const candidates = ['U','D','F','B','L','R'].filter(face => face !== previous); const face = candidates[Math.floor(Math.random() * candidates.length)]; const suffix = ['', "'", '2'][Math.floor(Math.random() * 3)]; const move = face + suffix; moves.push(move); applyMove(move, false); previous = face; } solutionStart = [...state]; solutionMoves = []; progress = 0; renderCube(); updateProgress(); setMessage(`Generated scramble: ${moves.join(' ')}`, true); };
function updateProgress() { document.querySelector('#progress').textContent = `${progress} / ${solutionMoves.length}`; document.querySelector('#current-move').textContent = progress ? solutionMoves[progress - 1] : '-'; document.querySelector('#progress-bar').style.width = solutionMoves.length ? `${progress / solutionMoves.length * 100}%` : '0%'; }
function stepTo(target) { target = Math.max(0, Math.min(solutionMoves.length, target)); state = [...solutionStart]; for (let i = 0; i < target; i++) applyMove(solutionMoves[i], false); progress = target; renderCube(); updateProgress(); }
function stop() { if (timer) clearInterval(timer); timer = null; document.querySelector('#play').textContent = 'Play'; }
document.querySelector('#first').onclick = () => stepTo(0); document.querySelector('#previous').onclick = () => stepTo(progress - 1); document.querySelector('#next').onclick = () => stepTo(progress + 1); document.querySelector('#last').onclick = () => stepTo(solutionMoves.length);
document.querySelector('#play').onclick = () => { if (timer) { stop(); return; } if (progress >= solutionMoves.length) stepTo(0); document.querySelector('#play').textContent = 'Pause'; timer = setInterval(() => { if (progress >= solutionMoves.length) stop(); else stepTo(progress + 1); }, 650); };

function loc(index) { const face = faces[Math.floor(index / 9)], local = index % 9, row = Math.floor(local / 3), col = local % 3; switch (face) { case 'U': return [[col - 1, 1, row - 1], [0, 1, 0]]; case 'D': return [[col - 1, -1, 1 - row], [0, -1, 0]]; case 'F': return [[col - 1, 1 - row, 1], [0, 0, 1]]; case 'B': return [[1 - col, 1 - row, -1], [0, 0, -1]]; case 'L': return [[-1, 1 - row, col - 1], [-1, 0, 0]]; default: return [[1, 1 - row, 1 - col], [1, 0, 0]]; } }
function rotate(vector, axis, direction) { let result = [...vector]; for (let i = 0; i < (direction < 0 ? 3 : 1); i++) { if (axis === 'x') result = [result[0], -result[2], result[1]]; if (axis === 'y') result = [result[2], result[1], -result[0]]; if (axis === 'z') result = [-result[1], result[0], result[2]]; } return result; }
function findIndex(position, normal) { for (let i = 0; i < 54; i++) { const candidate = loc(i); if (candidate[0].join() === position.join() && candidate[1].join() === normal.join()) return i; } return -1; }
function applyMove(move, rerender = true) { const face = move[0]; const turns = move[1] === '2' ? 2 : move[1] === "'" ? 3 : 1; const axis = face === 'U' || face === 'D' ? 'y' : face === 'F' || face === 'B' ? 'z' : 'x'; const normal = { U: [0,1,0], D: [0,-1,0], F: [0,0,1], B: [0,0,-1], L: [-1,0,0], R: [1,0,0] }[face]; const layer = normal[axis === 'x' ? 0 : axis === 'y' ? 1 : 2]; for (let turn = 0; turn < turns; turn++) { const old = [...state]; for (let i = 0; i < 54; i++) { const source = loc(i); const coordinate = source[0][axis === 'x' ? 0 : axis === 'y' ? 1 : 2]; if (coordinate !== layer) continue; const destination = findIndex(rotate(source[0], axis, -1), rotate(source[1], axis, -1)); state[destination] = old[i]; } } if (rerender) renderCube(); }
renderPalette(); renderCube(); updateProgress();

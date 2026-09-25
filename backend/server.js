const express = require('express');
const path = require('path');
const { spawnSync } = require('child_process');

const app = express();
const port = process.env.PORT || 3000;
const solverPath = process.env.SOLVER_PATH || path.join(__dirname, '..', 'build', 'solver.exe');
const repoRoot = path.resolve(__dirname, '..');
const wslSolverPath = process.env.WSL_SOLVER_PATH || `/mnt/${repoRoot[0].toLowerCase()}${repoRoot.slice(2).replaceAll('\\', '/')}/build/solver-wsl`;
app.use(express.json({ limit: '10kb' }));
app.use(express.static(path.join(__dirname, '..', 'frontend')));
app.use((request, response, next) => { response.setHeader('Access-Control-Allow-Origin', '*'); response.setHeader('Access-Control-Allow-Headers', 'Content-Type'); if (request.method === 'OPTIONS') return response.sendStatus(204); next(); });

function callSolver(action, cube) {
  const input = JSON.stringify({ action, cube });
  const useWsl = process.platform === 'win32' && process.env.SOLVER_RUNTIME !== 'native';
  const command = useWsl ? 'wsl.exe' : solverPath;
  const args = useWsl ? ['--', wslSolverPath] : [];
  const result = spawnSync(command, args, { input: `${input}\n`, encoding: 'utf8', timeout: 15000, windowsHide: true });
  if (result.error) {
    if (useWsl) throw new Error(`WSL solver unavailable: ${result.error.message}. Run npm run build:cpp:wsl first.`);
    if (result.error.code === 'UNKNOWN' || result.error.errno === -4094) {
      throw new Error(`Windows blocked the C++ solver executable. Use the WSL setup: npm run build:cpp:wsl, then restart the backend.`);
    }
    throw new Error(`Solver unavailable: ${result.error.message}`);
  }
  if (result.status !== 0) throw new Error(result.stderr || (useWsl ? 'WSL could not run the C++ solver. Run npm run build:cpp:wsl first.' : 'C++ solver exited unexpectedly.'));
  const output = result.stdout.trim();
  if (!output) throw new Error('C++ solver returned an empty response. Build the solver and check SOLVER_PATH.');
  const line = output.split(/\r?\n/).pop();
  return JSON.parse(line);
}
function readCube(request, response) {
  const cube = request.body && request.body.cube;
  if (typeof cube !== 'string' || cube.length !== 54) {
    response.status(400).json({ solvable: false, error: 'Cube state must be a 54-character string.' });
    return null;
  }
  return cube;
}
app.post('/api/validate', (request, response) => {
  const cube = readCube(request, response); if (!cube) return;
  try { response.json(callSolver('validate', cube)); } catch (error) { response.status(500).json({ solvable: false, error: error.message }); }
});
app.post('/api/solve', (request, response) => {
  const cube = readCube(request, response); if (!cube) return;
  try { response.json(callSolver('solve', cube)); } catch (error) { response.status(500).json({ solvable: false, error: error.message }); }
});
app.get('/api/health', (_request, response) => response.json({ ok: true, solver: solverPath }));
app.use((error, _request, response, _next) => response.status(500).json({ solvable: false, error: error.message }));
app.listen(port, () => console.log(`Rubik's Cube Solver running at http://localhost:${port}`));

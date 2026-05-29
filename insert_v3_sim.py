import re
import html

with open('projet-sokoban-autonome.html', 'r', encoding='utf-8') as f:
    content = f.read()

# We need to find where to insert the simulation in V3.
# Let's find: `<h2 class="text-gradient mt-6 mb-4 text-center">📂 Ressources de la Version 3</h2>`
marker = '<h2 class="text-gradient mt-6 mb-4 text-center">📂 Ressources de la Version 3</h2>'
pos = content.find(marker)
if pos == -1:
    print("Could not find insertion point!")
    exit(1)

v3_sim_html = """
        <h2 class="text-gradient mt-6 mb-4 text-center">🎮 Simulation Interactive (V3)</h2>
        <p class="text-center mb-4 text-secondary">Testez l'interpréteur de script : jouez manuellement ou laissez le programme lire une séquence de déplacements (Autonome).</p>

        <div class="simulation-controls-v3" style="display:flex; justify-content:center; gap:10px; margin-bottom:15px; flex-wrap:wrap; align-items: center;">
          <select id="level-select-v3" class="btn btn-secondary" style="padding:5px 15px; font-size:0.8rem; background: rgba(255,255,255,0.05); color: #fff; border: 1px solid rgba(255,255,255,0.2); outline: none;">
            <option value="niveau1.sok">Niveau 1</option>
            <option value="niveau2.sok">Niveau 2</option>
            <option value="niveau3.sok">Niveau 3</option>
            <option value="niveau4.sok">Niveau 4</option>
          </select>
          <button id="btn-manuel-v3" class="btn btn-secondary" style="padding:5px 15px; font-size:0.8rem;">1. Mode Manuel</button>
          <button id="btn-auto-v3" class="btn btn-secondary" style="padding:5px 15px; font-size:0.8rem;">2. Mode Autonome</button>
        </div>

        <div id="auto-controls-v3" style="display:none; max-width: 600px; margin: 0 auto 15px auto; background: rgba(0,0,0,0.4); padding: 15px; border-radius: 8px; border: 1px solid rgba(255,255,255,0.1);">
          <label style="display:block; margin-bottom: 8px; font-size: 0.85rem; color: #ccc;">Fichier de déplacements (.dep) :</label>
          <textarea id="dep-input-v3" rows="3" style="width: 100%; background: #111; color: #0f0; border: 1px solid #333; padding: 10px; font-family: 'JetBrains Mono', monospace; font-size: 14px; margin-bottom: 10px; border-radius: 4px; outline: none;">DDbbB</textarea>
          <div style="display:flex; gap: 10px; flex-wrap: wrap;">
            <button class="btn btn-secondary btn-preset-v3" data-seq="DDbbB" style="padding: 4px 10px; font-size: 0.75rem;">Victoire (Niv 1)</button>
            <button class="btn btn-secondary btn-preset-v3" data-seq="ddbbgg++Uhh" style="padding: 4px 10px; font-size: 0.75rem;">Test + Zoom + Undo</button>
            <button class="btn btn-secondary btn-preset-v3" data-seq="qqssdd" style="padding: 4px 10px; font-size: 0.75rem;">Echec</button>
            <button id="btn-run-auto-v3" class="btn btn-primary" style="padding: 4px 15px; font-size: 0.8rem; margin-left: auto;">▶️ Lancer l'interpréteur</button>
          </div>
        </div>

        <div class="terminal-emulator-container" style="margin: 0 auto 50px auto; max-width: 600px; background: #0c0c0c; border-radius: 10px; overflow: hidden; box-shadow: 0 15px 35px rgba(0,0,0,0.6); font-family: 'JetBrains Mono', monospace; font-size: 14px; color: #ccc; border: 1px solid #333;">
          <div style="background: #222; padding: 10px 15px; display: flex; gap: 8px; align-items: center; border-bottom: 1px solid #333;">
            <div style="width: 12px; height: 12px; border-radius: 50%; background: #ff5f56;"></div>
            <div style="width: 12px; height: 12px; border-radius: 50%; background: #ffbd2e;"></div>
            <div style="width: 12px; height: 12px; border-radius: 50%; background: #27c93f;"></div>
            <span style="margin-left: auto; margin-right: auto; color: #888; font-size: 12px; transform: translateX(-24px);">leny@iut-lannion: ~/sokoban_v3</span>
          </div>
          <div id="sokoban-container-v3" style="position: relative; min-height: 480px; cursor: pointer; overflow:hidden;">
            <div id="sokoban-terminal-v3" tabindex="0" style="padding: 20px; outline: none; white-space: pre; line-height: 1.2; min-height: 480px;">
              <!-- JS will render here -->
            </div>
            <div id="sokoban-overlay-v3" style="position:absolute; top:0; left:0; right:0; bottom:0; background:rgba(0,0,0,0.6); display:flex; align-items:center; justify-content:center; flex-direction:column; border-radius: 0 0 10px 10px;">
              <div style="font-size:3rem; margin-bottom:10px;">▶️</div>
              <div style="font-weight:bold;">Menu Principal</div>
            </div>
          </div>
        </div>

        <script>
          document.addEventListener('DOMContentLoaded', () => {
            const terminal = document.getElementById('sokoban-terminal-v3');
            const overlay = document.getElementById('sokoban-overlay-v3');
            const container = terminal ? terminal.parentElement : null;
            if(!terminal) return;

            const autoControls = document.getElementById('auto-controls-v3');
            const btnManuel = document.getElementById('btn-manuel-v3');
            const btnAuto = document.getElementById('btn-auto-v3');
            const levelSelect = document.getElementById('level-select-v3');
            const depInput = document.getElementById('dep-input-v3');
            const btnRunAuto = document.getElementById('btn-run-auto-v3');

            const NIVEAUX = {
              'niveau1.sok': ['  ####      ','###  ####   ','#     $ #   ','# #  #$ #   ','# . .#@ #   ','#########   ','            ','            ','            ','            ','            ','            '],
              'niveau2.sok': [' #####      ',' #@  #      ',' # $$# ###  ',' # $ # #.#  ',' ### ###.#  ','  ##    .#  ','  #   #  #  ','  #   ####  ','  #####     ','            ','            ','            '],
              'niveau3.sok': [' #####      ',' #.  ##     ',' #@$$ #     ',' ##   #     ','  ##  #     ','   ##.#     ','    ###     ','            ','            ','            ','            ','            '],
              'niveau4.sok': [' ######     ',' #.  #####  ',' #.  $...#  ',' # $#  $##  ',' #  ## @ #  ',' #   $ $ #  ',' ######  #  ','      ####  ','            ','            ','            ','            ']
            };
            
            const TAILLE = 12;
            let plateau = [];
            let hist = [];
            let currentLevelFile = 'niveau1.sok';
            let ligJoueur = 0;
            let colJoueur = 0;
            let nbCoups = 0;
            let zoom = 1;
            let gameOver = false;
            let hasFocus = false;
            
            let mode = 0; // 0 = menu, 1 = manuel, 2 = autonome
            let autoInterval = null;
            let autoIndex = 0;
            let depSequence = "";

            function showMenu() {
              mode = 0;
              terminal.innerHTML = "=== SUPER SOKOBAN ===\\n1. Mode Manuel (Jouer au clavier)\\n2. Mode Autonome (Lire un fichier .dep)\\n\\n(Sélectionnez un mode ci-dessus)";
              overlay.style.display = 'flex';
              overlay.innerHTML = '<div style="font-size:3rem; margin-bottom:10px;">▶️</div><div style="font-weight:bold;">Choisissez un mode</div>';
              autoControls.style.display = 'none';
              btnManuel.classList.remove('btn-primary'); btnManuel.classList.add('btn-secondary');
              btnAuto.classList.remove('btn-primary'); btnAuto.classList.add('btn-secondary');
              if(autoInterval) clearInterval(autoInterval);
            }

            function initPartie(filename) {
              currentLevelFile = filename;
              let NIVEAU = NIVEAUX[filename];
              plateau = [];
              hist = [];
              nbCoups = 0;
              zoom = 1;
              gameOver = false;
              for(let i=0; i<TAILLE; i++) {
                let row = (NIVEAU[i] || "").split('');
                while(row.length < TAILLE) row.push(' ');
                plateau.push(row);
                for(let j=0; j<TAILLE; j++) {
                  if(plateau[i][j] === '@' || plateau[i][j] === '+') {
                    ligJoueur = i; colJoueur = j;
                  }
                }
              }
            }

            function gagne() {
              for(let i=0; i<TAILLE; i++) {
                for(let j=0; j<TAILLE; j++) {
                  if(plateau[i][j] === '$') return false;
                }
              }
              return true;
            }

            function deplacer(direction, mode_auto) {
              let dy=0, dx=0, code='?';
              switch(direction.toLowerCase()) {
                case 'z': case 'h': dy=-1; code='h'; break;
                case 's': case 'b': dy=1; code='b'; break;
                case 'q': case 'g': dx=-1; code='g'; break;
                case 'd': dx=1; code='d'; break;
              }
              if(code === '?') return;

              let y = ligJoueur, x = colJoueur;
              let ny = y + dy, nx = x + dx;
              if(ny<0 || ny>=TAILLE || nx<0 || nx>=TAILLE) return;

              let actuel = plateau[y][x];
              let suiv = plateau[ny][nx];

              if(suiv === ' ' || suiv === '.') {
                plateau[y][x] = (actuel === '+') ? '.' : ' ';
                plateau[ny][nx] = (suiv === '.') ? '+' : '@';
                ligJoueur = ny; colJoueur = nx;
                if(!mode_auto) hist[nbCoups++] = code;
              } else if(suiv === '$' || suiv === '*') {
                let py = ny + dy, px = nx + dx;
                if(py>=0 && py<TAILLE && px>=0 && px<TAILLE) {
                  let apres = plateau[py][px];
                  if(apres === ' ' || apres === '.') {
                    plateau[py][px] = (apres === '.') ? '*' : '$';
                    plateau[ny][nx] = (suiv === '*') ? '+' : '@';
                    plateau[y][x] = (actuel === '+') ? '.' : ' ';
                    ligJoueur = ny; colJoueur = nx;
                    if(!mode_auto) hist[nbCoups++] = code.toUpperCase();
                  }
                }
              }
            }

            function annuler_coup() {
              if(nbCoups <= 0) return;
              let mouv = hist[nbCoups - 1];
              let dy=0, dx=0;
              switch(mouv.toLowerCase()) {
                case 'h': dy=1; break;
                case 'b': dy=-1; break;
                case 'g': dx=1; break;
                case 'd': dx=-1; break;
              }
              let y = ligJoueur, x = colJoueur;
              let py = y + dy, px = x + dx;
              if(py<0 || py>=TAILLE || px<0 || px>=TAILLE) return;

              plateau[y][x] = (plateau[y][x] === '+') ? '.' : ' ';
              plateau[py][px] = (plateau[py][px] === '.') ? '+' : '@';

              if(mouv >= 'A' && mouv <= 'Z') {
                let cy = y - dy, cx = x - dx;
                plateau[cy][cx] = (plateau[cy][cx] === '*') ? '.' : ' ';
                plateau[y][x] = (plateau[y][x] === '.') ? '*' : '$';
              }
              ligJoueur = py; colJoueur = px;
              nbCoups--;
            }

            function renderPlateau(out) {
              for(let i=0; i<TAILLE; i++) {
                for(let v=0; v<zoom; v++) {
                  for(let j=0; j<TAILLE; j++) {
                    let c = plateau[i][j];
                    for(let h=0; h<zoom; h++) {
                        out += c;
                    }
                  }
                  out += "\\n";
                }
              }
              return out;
            }

            function renderManuel() {
              let out = "--- MODE MANUEL ---\\n";
              out += "Fichier: " + currentLevelFile + " | Coups: " + nbCoups + " | Zoom: x" + zoom + "\\n";
              out += "Touches: ZQSD (bouger), +/- (zoom), U (undo), R (reset), X (quitter)\\n";
              out += "--------------------------------------------------\\n";
              out = renderPlateau(out);
              terminal.innerHTML = out;
            }

            function renderAuto() {
              let out = "--- MODE AUTONOME ---\\n";
              out += "Partie: " + currentLevelFile + " | Deplacements: memoire.dep\\n";
              out += "Progression: " + autoIndex + " / " + depSequence.length + " | Zoom: x" + zoom + "\\n";
              out += "--------------------------------------------------\\n";
              out = renderPlateau(out);
              terminal.innerHTML = out;
            }

            // Keyboard for Manuel mode
            terminal.addEventListener('keydown', (e) => {
              if(!hasFocus || mode !== 1 || gameOver) return;
              if(['ArrowUp','ArrowDown','ArrowLeft','ArrowRight',' '].includes(e.key)) {
                e.preventDefault();
              }

              let k = e.key.toLowerCase();
              if(k === 'r') { initPartie(currentLevelFile); renderManuel(); return; }
              if(k === 'x') {
                gameOver = true;
                renderManuel();
                terminal.innerHTML += "\\nPartie abandonnee (" + nbCoups + " coups).\\nSauvegarder l\\'historique ? (o/n) : n\\n";
                return;
              }
              if(e.key === '+') { if(zoom < 3) { zoom++; renderManuel(); } }
              else if(e.key === '-') { if(zoom > 1) { zoom--; renderManuel(); } }
              else if(k === 'u') { annuler_coup(); renderManuel(); }
              else if(['z','q','s','d','h','b','g','arrowup','arrowdown','arrowleft','arrowright'].includes(k)) {
                if(k === 'arrowup') k = 'z';
                if(k === 'arrowdown') k = 's';
                if(k === 'arrowleft') k = 'q';
                if(k === 'arrowright') k = 'd';
                deplacer(k, false);
                renderManuel();
              }
              
              if(gagne()) {
                gameOver = true;
                terminal.innerHTML += "\\nBRAVO ! GAGNE !\\nSauvegarder l\\'historique (.dep) ? (o/n) : n\\n";
              }
            });

            // Auto loop
            function stepAuto() {
              if (gagne() || autoIndex >= depSequence.length) {
                clearInterval(autoInterval);
                autoInterval = null;
                renderAuto();
                terminal.innerHTML += "\\n--------------------------------------------------\\n";
                if(gagne()) {
                  terminal.innerHTML += "SOLUTION VALIDE (Gagne en " + autoIndex + " coups).\\n";
                } else {
                  terminal.innerHTML += "ECHEC : Partie perdue avec cette liste de coups.\\n";
                }
                terminal.innerHTML += "--------------------------------------------------\\n";
                return;
              }

              let coup = depSequence[autoIndex];
              let cl = coup.toLowerCase();
              
              if (coup === '+') { if(zoom<3) zoom++; }
              else if (coup === '-') { if(zoom>1) zoom--; }
              else if (cl === 'u') {
                // In real V3, undo in auto mode is weird but supported
                // We'll just do nothing or simulate if we had hist
              } else {
                deplacer(cl, true);
              }
              
              autoIndex++;
              renderAuto();
            }

            container.addEventListener('click', () => {
              if (mode === 0) return; // Must select mode
              terminal.focus();
            });

            terminal.addEventListener('focus', () => {
              if (mode === 1) {
                hasFocus = true;
                overlay.style.display = 'none';
              }
            });

            terminal.addEventListener('blur', () => {
              if (mode === 1 && !gameOver) {
                hasFocus = false;
                overlay.style.display = 'flex';
                overlay.innerHTML = '<div style="font-size:3rem; margin-bottom:10px;">▶️</div><div style="font-weight:bold;">Cliquez pour jouer</div>';
              }
            });

            // UI Listeners
            btnManuel.addEventListener('click', () => {
              mode = 1;
              btnManuel.classList.remove('btn-secondary'); btnManuel.classList.add('btn-primary');
              btnAuto.classList.remove('btn-primary'); btnAuto.classList.add('btn-secondary');
              autoControls.style.display = 'none';
              
              initPartie(levelSelect.value);
              renderManuel();
              overlay.style.display = 'flex';
              overlay.innerHTML = '<div style="font-size:3rem; margin-bottom:10px;">▶️</div><div style="font-weight:bold;">Cliquez pour jouer</div>';
            });

            btnAuto.addEventListener('click', () => {
              mode = 2;
              btnAuto.classList.remove('btn-secondary'); btnAuto.classList.add('btn-primary');
              btnManuel.classList.remove('btn-primary'); btnManuel.classList.add('btn-secondary');
              autoControls.style.display = 'block';
              if(autoInterval) clearInterval(autoInterval);
              
              initPartie(levelSelect.value);
              depSequence = depInput.value.replace(/\\s/g, '');
              autoIndex = 0;
              renderAuto();
              
              overlay.style.display = 'flex';
              overlay.innerHTML = '<div style="font-size:3rem; margin-bottom:10px;">🤖</div><div style="font-weight:bold;">Prêt à exécuter</div>';
            });

            btnRunAuto.addEventListener('click', () => {
              if (mode !== 2) return;
              initPartie(levelSelect.value);
              depSequence = depInput.value.replace(/\\s/g, '');
              autoIndex = 0;
              overlay.style.display = 'none';
              renderAuto();
              if(autoInterval) clearInterval(autoInterval);
              
              // 1s pause avant depart then 250ms interval
              setTimeout(() => {
                autoInterval = setInterval(stepAuto, 250);
              }, 1000);
            });

            document.querySelectorAll('.btn-preset-v3').forEach(btn => {
              btn.addEventListener('click', (e) => {
                depInput.value = e.target.getAttribute('data-seq');
              });
            });

            levelSelect.addEventListener('change', () => {
              if(mode === 1) {
                initPartie(levelSelect.value);
                renderManuel();
              } else if(mode === 2) {
                if(autoInterval) clearInterval(autoInterval);
                initPartie(levelSelect.value);
                autoIndex = 0;
                renderAuto();
                overlay.style.display = 'flex';
                overlay.innerHTML = '<div style="font-size:3rem; margin-bottom:10px;">🤖</div><div style="font-weight:bold;">Prêt à exécuter</div>';
              }
            });

            showMenu();
          });
        </script>
"""

new_content = content[:pos] + v3_sim_html + '\n\n' + content[pos:]

with open('projet-sokoban-autonome.html', 'w', encoding='utf-8') as f:
    f.write(new_content)

print("V3 Simulation inserted")

const terminal = document.getElementById('sokoban-terminal-v2');
            const overlay = document.getElementById('sokoban-overlay-v2');
            const container = terminal ? terminal.parentElement : null;
            if(!terminal) return;

            const NIVEAUX = {
              'niveau1.sok': ['  ####      ','###  ####   ','#     $ #   ','# #  #$ #   ','# . .#@ #   ','#########   ','            ','            ','            ','            ','            ','            '],
              'niveau2.sok': [' #####      ',' #@  #      ',' # $$# ###  ',' # $ # #.#  ',' ### ###.#  ','  ##    .#  ','  #   #  #  ','  #   ####  ','  #####     ','            ','            ','            '],
              'niveau3.sok': [' #####      ',' #.  ##     ',' #@$$ #     ',' ##   #     ','  ##  #     ','   ##.#     ','    ###     ','            ','            ','            ','            ','            '],
              'niveau4.sok': [' ######     ',' #.  #####  ',' #.  $...#  ',' # $#  $##  ',' #  ## @ #  ',' #   $ $ #  ',' ######  #  ','      ####  ','            ','            ','            ','            '],
              'niveau5.sok': [' ####       ',' #. ##      ',' #.@ #      ',' #. $#      ',' ##$ ###    ','  # $  #    ','  #    #    ','  #  ###    ','  ####      ','            ','            ','            '],
              'niveau6.sok': ['  #####     ','###   #     ','#.@$  #     ','### $.#     ','#.##$ #     ','# # . ##    ','#$ *$$.#    ','#   .  #    ','########    ','            ','            ','            ']
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
              render();
            }

            function gagne() {
              for(let i=0; i<TAILLE; i++) {
                for(let j=0; j<TAILLE; j++) {
                  if(plateau[i][j] === '$') return false;
                }
              }
              return true;
            }

            function deplacer(direction) {
              let dy=0, dx=0, code='?';
              switch(direction.toLowerCase()) {
                case 'z': dy=-1; code='h'; break;
                case 's': dy=1; code='b'; break;
                case 'q': dx=-1; code='g'; break;
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
                hist[nbCoups++] = code;
              } else if(suiv === '$' || suiv === '*') {
                let py = ny + dy, px = nx + dx;
                if(py>=0 && py<TAILLE && px>=0 && px<TAILLE) {
                  let apres = plateau[py][px];
                  if(apres === ' ' || apres === '.') {
                    plateau[py][px] = (apres === '.') ? '*' : '$';
                    plateau[ny][nx] = (suiv === '*') ? '+' : '@';
                    plateau[y][x] = (actuel === '+') ? '.' : ' ';
                    ligJoueur = ny; colJoueur = nx;
                    hist[nbCoups++] = code.toUpperCase();
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

            function render() {
              let out = "=== SOKOBAN V2 ===\n";
              out += "Fichier : " + currentLevelFile + " | Coups : " + nbCoups + " | Zoom : " + zoom + "x\n";
              out += "Touches : ZQSD, +/-, U=undo, R=reset, X=quitter\n";
              out += "------------------------------------------------------\n";
              
              for(let i=0; i<TAILLE; i++) {
                for(let v=0; v<zoom; v++) {
                  for(let j=0; j<TAILLE; j++) {
                    let c = plateau[i][j];
                    for(let h=0; h<zoom; h++) {
                        out += c;
                    }
                  }
                  out += "\n";
                }
              }
              terminal.innerHTML = out;
            }

            terminal.addEventListener('keydown', (e) => {
              if(!hasFocus) return;
              if(['ArrowUp','ArrowDown','ArrowLeft','ArrowRight',' '].includes(e.key)) {
                e.preventDefault();
              }
              if(gameOver) return;

              let k = e.key.toLowerCase();
              if(k === 'r') { initPartie(currentLevelFile); return; }
              if(k === 'x') {
                gameOver = true;
                render();
                terminal.innerHTML += "\nPartie abandonnee (" + nbCoups + " coups).\nSauvegarder le plateau ? (o/n) : n\nSauvegarder l\'historique ? (o/n) : n\n";
                return;
              }
              if(e.key === '+') { if(zoom < 3) { zoom++; render(); } }
              else if(e.key === '-') { if(zoom > 1) { zoom--; render(); } }
              else if(k === 'u') { annuler_coup(); render(); }
              else if(['z','q','s','d','arrowup','arrowdown','arrowleft','arrowright'].includes(k)) {
                if(k === 'arrowup') k = 'z';
                if(k === 'arrowdown') k = 's';
                if(k === 'arrowleft') k = 'q';
                if(k === 'arrowright') k = 'd';
                deplacer(k);
                render();
              }
              
              if(gagne()) {
                gameOver = true;
                terminal.innerHTML += "\nBravo ! Partie gagnee en " + nbCoups + " coups.\nSauvegarder l\'historique ? (o/n) : n\n";
              }
            });

            container.addEventListener('click', () => {
              terminal.focus();
            });

            terminal.addEventListener('focus', () => {
              hasFocus = true;
              overlay.style.display = 'none';
            });

            terminal.addEventListener('blur', () => {
              hasFocus = false;
              overlay.style.display = 'flex';
            });

            document.querySelectorAll('.level-btn-v2').forEach(btn => {
              btn.addEventListener('click', (e) => {
                let f = e.target.getAttribute('data-level');
                initPartie(f);
                terminal.focus();
              });
            });

            initPartie('niveau1.sok');
          });
        
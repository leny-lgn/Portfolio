          document.addEventListener('DOMContentLoaded', () => {
            const terminal = {};
            const overlay = {};
            const container = {};

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
            
            let mode = 0; 
            let autoInterval = null;
            let autoIndex = 0;
            let depSequence = "";

            function showMenu() {
              mode = 0;
              terminal.innerHTML = "=== SUPER SOKOBAN ===\n1. Mode Manuel (Jouer au clavier)\n2. Mode Autonome (Lire un fichier .dep)\n\n(Sélectionnez un mode ci-dessus)";
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

          });

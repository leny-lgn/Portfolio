document = {
  addEventListener: function() {},
  getElementById: function() { return {}; },
  querySelectorAll: function() { return []; }
};
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
      }}
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
  // terminal.innerHTML = out;
}

// ... event listeners ...

// Main JS logic
document.addEventListener("DOMContentLoaded", () => {
  
  // Mobile check
  const isMobile = window.matchMedia("(max-width: 768px)").matches;
  const isTouch = window.matchMedia("(hover: none)").matches;

  // --- GSAP Setup ---
  gsap.registerPlugin(ScrollTrigger);

  // --- Preloader ---
  const preloader = document.getElementById('preloader');
  if (preloader && !sessionStorage.getItem('siteLoaded')) {
    let progress = 0;
    const counterEl = document.querySelector('#preloader .counter');
    const interval = setInterval(() => {
      progress += Math.floor(Math.random() * 10) + 1;
      if (progress > 100) progress = 100;
      if(counterEl) counterEl.textContent = progress + '%';
      if (progress === 100) {
        clearInterval(interval);
        gsap.to(preloader, {
          yPercent: -100,
          duration: 1,
          ease: "power3.inOut",
          onComplete: () => {
            sessionStorage.setItem('siteLoaded', 'true');
            initHeroAnimations();
          }
        });
      }
    }, 50);
  } else if (preloader) {
    preloader.style.display = 'none';
    initHeroAnimations();
  } else {
    initHeroAnimations();
  }

  // --- Scroll Progress Bar ---
  const progressBar = document.querySelector('.scroll-progress');
  if(progressBar) {
    window.addEventListener('scroll', () => {
      const scrollPx = document.documentElement.scrollTop;
      const winHeightPx = document.documentElement.scrollHeight - document.documentElement.clientHeight;
      const scrolled = `${(scrollPx / winHeightPx) * 100}%`;
      progressBar.style.transform = `scaleX(${scrollPx / winHeightPx})`;
    });
  }

  // --- Header Shrink & Glass ---
  const header = document.querySelector('header');
  if(header) {
    window.addEventListener('scroll', () => {
      if (window.scrollY > 80) {
        header.classList.add('scrolled');
      } else {
        header.classList.remove('scrolled');
      }
    });
  }


  // --- Hero Animations ---
  function initHeroAnimations() {
    // Text Reveal
    const lines = document.querySelectorAll('.hero-title .line');
    if(lines.length > 0) {
      gsap.to(lines, {
        y: '0%',
        duration: 1,
        stagger: 0.12,
        ease: "power3.out"
      });
    }

    // Role Cycling
    const roles = document.querySelectorAll('.hero-role');
    if (roles.length > 0) {
      let currentRole = 0;
      roles[0].classList.add('active');
      setInterval(() => {
        roles[currentRole].classList.remove('active');
        roles[currentRole].classList.add('exit');
        setTimeout(()=>{
          roles[currentRole].classList.remove('exit');
        }, 500);

        currentRole = (currentRole + 1) % roles.length;
        roles[currentRole].classList.add('active');
      }, 2500);
    }
  }

  // --- ScrollTrigger Animations ---
  const fadeElements = document.querySelectorAll('.fade-in-up');
  fadeElements.forEach(el => {
    gsap.fromTo(el, 
      { y: 30, opacity: 0 },
      {
        y: 0, opacity: 1, duration: 0.6,
        scrollTrigger: {
          trigger: el,
          start: "top 85%",
          toggleActions: "play none none reverse"
        }
      }
    );
  });

  const cards = document.querySelectorAll('.stagger-cards .card');
  if(cards.length > 0) {
    gsap.fromTo(cards,
      { y: 50, opacity: 0 },
      {
        y: 0, opacity: 1, duration: 0.5, stagger: 0.1,
        scrollTrigger: {
          trigger: '.stagger-cards',
          start: "top 85%"
        }
      }
    );
  }

  // --- Hero Parallax ---
  const heroParallax = document.querySelector('.hero-parallax');
  if (heroParallax && !isMobile) {
    gsap.to(heroParallax, {
      yPercent: 30,
      ease: "none",
      scrollTrigger: {
        trigger: ".hero",
        start: "top top",
        end: "bottom top",
        scrub: true
      }
    });
  }

  // --- Animated Counters ---
  const counters = document.querySelectorAll('.counter-val');
  counters.forEach(counter => {
    ScrollTrigger.create({
      trigger: counter,
      start: "top 85%",
      once: true,
      onEnter: () => {
        let target = parseInt(counter.getAttribute('data-target'));
        gsap.to(counter, {
          innerHTML: target,
          duration: 1.5,
          ease: "power2.out",
          snap: { innerHTML: 1 }
        });
      }
    });
  });

  // --- Timeline (Page Parcours) ---
  const timeline = document.querySelector('.timeline');
  if (timeline) {
    // Line draw
    gsap.to(timeline, {
      '--height': '100%',
      ease: "none",
      scrollTrigger: {
        trigger: timeline,
        start: "top 50%",
        end: "bottom 50%",
        scrub: true,
        onUpdate: self => {
          timeline.style.setProperty('--height', `${self.progress * 100}%`);
          timeline.style.setProperty('height', `${self.progress * 100}%`); // fallback
        }
      }
    });

    const items = document.querySelectorAll('.timeline-item');
    items.forEach((item, index) => {
      let xOffset = index % 2 === 0 ? -50 : 50;
      if (isMobile) xOffset = 50;
      
      gsap.fromTo(item, 
        { x: xOffset, opacity: 0 },
        {
          x: 0, opacity: 1, duration: 0.6,
          scrollTrigger: {
            trigger: item,
            start: "top 85%"
          }
        }
      );
      
      const dot = item.querySelector('.timeline-dot');
      if (dot) {
        gsap.to(dot, {
          scale: 1, duration: 0.4, delay: 0.2,
          scrollTrigger: { trigger: item, start: "top 85%" }
        });
      }
    });
  }

  // --- Interactive CV ---
  const cvSections = document.querySelectorAll('.cv-section');
  const cvDoc = document.querySelector('.cv-document');
  const cvPanel = document.querySelector('.cv-detail-panel');
  const closePanelBtn = document.querySelector('.close-panel');

  if(cvSections.length > 0 && cvDoc && cvPanel) {
    cvSections.forEach(sec => {
      sec.addEventListener('click', () => {
        const targetId = sec.getAttribute('data-target');
        
        // Hide all details
        document.querySelectorAll('.cv-detail-content').forEach(d => d.style.display = 'none');
        // Show target
        const content = document.getElementById(targetId);
        if(content) content.style.display = 'block';

        // Animate layouts
        cvDoc.classList.add('shrink');
        cvPanel.classList.add('active');
      });
    });

    if(closePanelBtn) {
      closePanelBtn.addEventListener('click', () => {
        cvDoc.classList.remove('shrink');
        cvPanel.classList.remove('active');
      });
    }
  }

  // --- Tilt 3D on Cards ---
  if (!isMobile && !isTouch) {
    const tiltCards = document.querySelectorAll('.tilt-card');
    tiltCards.forEach(card => {
      card.addEventListener('mousemove', (e) => {
        const rect = card.getBoundingClientRect();
        const x = e.clientX - rect.left;
        const y = e.clientY - rect.top;
        const centerX = rect.width / 2;
        const centerY = rect.height / 2;
        
        const rotateX = ((y - centerY) / centerY) * -10;
        const rotateY = ((x - centerX) / centerX) * 10;
        
        card.style.transform = `perspective(800px) rotateX(${rotateX}deg) rotateY(${rotateY}deg) scale3d(1.02, 1.02, 1.02)`;
      });
      
      card.addEventListener('mouseleave', () => {
        card.style.transform = `perspective(800px) rotateX(0deg) rotateY(0deg) scale3d(1, 1, 1)`;
      });
    });
  }

  // --- Text Scramble ---
  class TextScramble {
    constructor(el) {
      this.el = el;
      this.chars = 'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789@#$%';
      this.update = this.update.bind(this);
    }
    setText(newText) {
      const oldText = this.el.innerText;
      const length = Math.max(oldText.length, newText.length);
      const promise = new Promise((resolve) => this.resolve = resolve);
      this.queue = [];
      for (let i = 0; i < length; i++) {
        const from = oldText[i] || '';
        const to = newText[i] || '';
        const start = Math.floor(Math.random() * 40);
        const end = start + Math.floor(Math.random() * 40);
        this.queue.push({ from, to, start, end });
      }
      cancelAnimationFrame(this.frameRequest);
      this.frame = 0;
      this.update();
      return promise;
    }
    update() {
      let output = '';
      let complete = 0;
      for (let i = 0, n = this.queue.length; i < n; i++) {
        let { from, to, start, end, char } = this.queue[i];
        if (this.frame >= end) {
          complete++;
          output += to;
        } else if (this.frame >= start) {
          if (!char || Math.random() < 0.28) {
            char = this.randomChar();
            this.queue[i].char = char;
          }
          output += `<span class="scramble-char">${char}</span>`;
        } else {
          output += from;
        }
      }
      this.el.innerHTML = output;
      if (complete === this.queue.length) {
        this.resolve();
      } else {
        this.frameRequest = requestAnimationFrame(this.update);
        this.frame++;
      }
    }
    randomChar() {
      return this.chars[Math.floor(Math.random() * this.chars.length)];
    }
  }

  const scrambleEls = document.querySelectorAll('.scramble-text');
  scrambleEls.forEach(el => {
    const text = el.getAttribute('data-text') || el.innerText;
    el.innerHTML = '';
    const fx = new TextScramble(el);
    
    ScrollTrigger.create({
      trigger: el,
      start: "top 90%",
      once: true,
      onEnter: () => fx.setText(text)
    });
  });

  // --- Hero Canvas Particles ---
  const canvas = document.getElementById('hero-canvas');
  if (canvas && !isMobile) {
    const ctx = canvas.getContext('2d');
    canvas.width = window.innerWidth;
    canvas.height = window.innerHeight;
    
    let particlesArray = [];
    const numParticles = 100;

    class Particle {
      constructor() {
        this.x = Math.random() * canvas.width;
        this.y = Math.random() * canvas.height;
        this.size = Math.random() * 2;
        this.speedX = Math.random() * 1 - 0.5;
        this.speedY = Math.random() * 1 - 0.5;
      }
      update() {
        this.x += this.speedX;
        this.y += this.speedY;
        
        if(this.x < 0 || this.x > canvas.width) this.speedX *= -1;
        if(this.y < 0 || this.y > canvas.height) this.speedY *= -1;
      }
      draw() {
        ctx.fillStyle = 'rgba(255, 255, 255, 0.5)';
        ctx.beginPath();
        ctx.arc(this.x, this.y, this.size, 0, Math.PI * 2);
        ctx.fill();
      }
    }

    for (let i = 0; i < numParticles; i++) {
      particlesArray.push(new Particle());
    }

    function animateParticles() {
      ctx.clearRect(0, 0, canvas.width, canvas.height);
      for (let i = 0; i < particlesArray.length; i++) {
        particlesArray[i].update();
        particlesArray[i].draw();
        
        for (let j = i; j < particlesArray.length; j++) {
          const dx = particlesArray[i].x - particlesArray[j].x;
          const dy = particlesArray[i].y - particlesArray[j].y;
          const distance = Math.sqrt(dx * dx + dy * dy);
          
          if (distance < 120) {
            ctx.beginPath();
            ctx.strokeStyle = `rgba(255, 255, 255, ${0.2 - distance/600})`;
            ctx.lineWidth = 0.5;
            ctx.moveTo(particlesArray[i].x, particlesArray[i].y);
            ctx.lineTo(particlesArray[j].x, particlesArray[j].y);
            ctx.stroke();
          }
        }
      }
      requestAnimationFrame(animateParticles);
    }
    animateParticles();
    
    window.addEventListener('resize', () => {
      canvas.width = window.innerWidth;
      canvas.height = window.innerHeight;
    });
  } else if (canvas) {
    canvas.remove();
  }

});

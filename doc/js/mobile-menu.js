// Mobile Navigation Implementation for Xnec2c Documentation
document.addEventListener('DOMContentLoaded', function() {
  // Create mobile menu toggle button
  const menuToggle = document.createElement('button');
  menuToggle.id = 'mobile-menu-toggle';
  menuToggle.innerHTML = `
    <span class="menu-icon">
      <span></span>
      <span></span>
      <span></span>
    </span>
    <span class="menu-text">Menu</span>
  `;
  menuToggle.setAttribute('aria-label', 'Toggle navigation menu');
  menuToggle.setAttribute('aria-expanded', 'false');

  // Create mobile navigation container
  const mobileNav = document.createElement('div');
  mobileNav.id = 'mobile-nav-container';
  mobileNav.className = 'mobile-nav-closed';

  // Find existing navigation
  const existingNav = document.querySelector('nav[role="navigation"]');
  if (existingNav) {
    // Clone the navigation for mobile
    const navClone = existingNav.cloneNode(true);
    navClone.className = 'mobile-nav';

    // Ensure the navigation is visible in mobile menu
    navClone.style.display = 'block';

    // Add close button inside navigation
    const closeBtn = document.createElement('button');
    closeBtn.className = 'nav-close';
    closeBtn.innerHTML = '×';
    closeBtn.setAttribute('aria-label', 'Close navigation');

    mobileNav.appendChild(closeBtn);
    mobileNav.appendChild(navClone);

    // Insert mobile menu elements at the top of body
    document.body.insertBefore(menuToggle, document.body.firstChild);
    document.body.insertBefore(mobileNav, menuToggle.nextSibling);

    // Create overlay for mobile menu
    const overlay = document.createElement('div');
    overlay.id = 'mobile-nav-overlay';
    overlay.className = 'overlay-hidden';
    document.body.insertBefore(overlay, mobileNav.nextSibling);

    // Toggle menu function
    function toggleMenu() {
      const isOpen = mobileNav.classList.contains('mobile-nav-open');

      if (isOpen) {
        // Close menu
        mobileNav.classList.remove('mobile-nav-open');
        mobileNav.classList.add('mobile-nav-closed');
        overlay.classList.remove('overlay-visible');
        overlay.classList.add('overlay-hidden');
        menuToggle.setAttribute('aria-expanded', 'false');
        document.body.style.overflow = '';
      } else {
        // Open menu
        mobileNav.classList.remove('mobile-nav-closed');
        mobileNav.classList.add('mobile-nav-open');
        overlay.classList.remove('overlay-hidden');
        overlay.classList.add('overlay-visible');
        menuToggle.setAttribute('aria-expanded', 'true');
        document.body.style.overflow = 'hidden';
      }
    }

    // Event listeners
    menuToggle.addEventListener('click', toggleMenu);
    closeBtn.addEventListener('click', toggleMenu);
    overlay.addEventListener('click', toggleMenu);

    // Close menu when clicking on a link (for same-page navigation)
    mobileNav.querySelectorAll('a[href^="#"]').forEach(link => {
      link.addEventListener('click', function() {
        setTimeout(toggleMenu, 100); // Small delay for smooth transition
      });
    });

    // Handle escape key
    document.addEventListener('keydown', function(e) {
      if (e.key === 'Escape' && mobileNav.classList.contains('mobile-nav-open')) {
        toggleMenu();
      }
    });
  }

  // Add "Back to Top" button
  const backToTop = document.createElement('button');
  backToTop.id = 'back-to-top';
  backToTop.innerHTML = '↑';
  backToTop.setAttribute('aria-label', 'Back to top');
  backToTop.style.display = 'none';
  document.body.appendChild(backToTop);

  // Show/hide back to top based on scroll
  let scrollTimeout;
  window.addEventListener('scroll', function() {
    clearTimeout(scrollTimeout);
    scrollTimeout = setTimeout(function() {
      if (window.pageYOffset > 300) {
        backToTop.style.display = 'block';
      } else {
        backToTop.style.display = 'none';
      }
    }, 100);
  });

  backToTop.addEventListener('click', function() {
    window.scrollTo({ top: 0, behavior: 'smooth' });
  });

  // Improve table responsiveness
  const tables = document.querySelectorAll('table');
  tables.forEach(table => {
    if (!table.closest('.table-wrapper')) {
      const wrapper = document.createElement('div');
      wrapper.className = 'table-wrapper';
      table.parentNode.insertBefore(wrapper, table);
      wrapper.appendChild(table);
    }
  });

  // Add horizontal scroll indicators for code blocks
  const codeBlocks = document.querySelectorAll('pre');
  codeBlocks.forEach(pre => {
    if (pre.scrollWidth > pre.clientWidth) {
      pre.classList.add('has-horizontal-scroll');
    }
  });

  // Handle window resize
  let resizeTimeout;
  window.addEventListener('resize', function() {
    clearTimeout(resizeTimeout);
    resizeTimeout = setTimeout(function() {
      // Close mobile menu if window becomes wide
      if (window.innerWidth > 768 && mobileNav.classList.contains('mobile-nav-open')) {
        toggleMenu();
      }

      // Recheck code blocks for horizontal scroll
      codeBlocks.forEach(pre => {
        if (pre.scrollWidth > pre.clientWidth) {
          pre.classList.add('has-horizontal-scroll');
        } else {
          pre.classList.remove('has-horizontal-scroll');
        }
      });
    }, 250);
  });
});
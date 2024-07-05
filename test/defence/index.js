document.addEventListener('DOMContentLoaded', (event) => {
    document.getElementById('scroll-left').addEventListener('click', function () {
        document.querySelector('.releases').scrollBy({
            left: -300, // Adjust this value based on the width of .release-item
            behavior: 'smooth'
        });
    });

    document.getElementById('scroll-right').addEventListener('click', function () {
        document.querySelector('.releases').scrollBy({
            left: 300, // Adjust this value based on the width of .release-item
            behavior: 'smooth'
        });
    });
});

// Function to load content into the container
function loadContent(page, buttonElement) {
    const contentContainer = document.getElementById('content-container');
    contentContainer.style.display = 'block';

    // Fade out current content
    contentContainer.style.opacity = 0;
    contentContainer.style.transition = 'opacity 0.2s';

    // Wait for the fade-out to complete
    setTimeout(() => {
        fetch(page)
            .then(response => response.text())
            .then(data => {
                const releaseItem = buttonElement.closest('.release-item');
                const bgColor = window.getComputedStyle(releaseItem).backgroundColor;
                contentContainer.innerHTML = data;
                contentContainer.style.backgroundColor = bgColor;

                // Fade in new content
                contentContainer.style.opacity = 1;

                // Dynamically load JavaScript file for the loaded content
                if (page === 'methods.html') {
                    const script = document.createElement('script');
                    script.src = 'methods.js';
                    document.body.appendChild(script);
                }
            })
            .catch(error => console.error('Error loading content:', error));
    }, 500); // Match this delay with the fade-out duration
}

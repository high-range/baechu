document.addEventListener('DOMContentLoaded', (event) => {
    console.log('DOM fully loaded and parsed');

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

let currentPage = '';  // 현재 로드된 페이지를 추적하기 위한 변수

// Function to load content into the container
function loadContent(page, buttonElement) {
    if (currentPage === page) {
        console.log(`Page ${page} is already loaded.`);
        return;  // 현재 페이지가 이미 로드된 상태이면 fetch를 호출하지 않음
    }
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
                    if (document.querySelector('script[src="methods.js"]'))
                    {
                        document.getElementsByTagName('script')[0].remove();
                        console.log('methods.js script removed');
                    }
                    const script = document.createElement('script');
                    script.src = 'methods.js';
                    script.onload = () => console.log('methods.js script loaded');
                    document.body.appendChild(script);
                } else if (page === 'cgi.html') {
                    if (document.querySelector('script[src="cgi.js"]'))
                    {
                        document.getElementsByTagName('script')[0].remove();
                        console.log('cgi.js script removed');
                    }
                    const script = document.createElement('script');
                    script.src = 'cgi.js';
                    script.onload = () => console.log('cgi.js script loaded');
                    document.body.appendChild(script);
                } else if (page === 'features.html') {
                    if (document.querySelector('script[src="features.js"]'))
                    {
                        document.getElementsByTagName('script')[0].remove();
                        console.log('features.js script removed');
                    }
                    const script = document.createElement('script');
                    script.src = 'features.js';
                    script.onload = () => console.log('features.js script loaded');
                    document.body.appendChild(script);
                }

                // Update the current page after successful fetch and script loading
                currentPage = page;
            })
            .catch(error => console.error('Error loading content:', error));
    }, 500); // Match this delay with the fade-out duration
}

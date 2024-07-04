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

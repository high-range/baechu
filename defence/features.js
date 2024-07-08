function toggleCard(card) {
    if (card.classList.contains('active')) {
        card.classList.remove('active');
        card.classList.add('deactivating');
        card.addEventListener('animationend', function () {
            card.classList.remove('deactivating');
        }, { once: true });
    } else {
        card.classList.add('active');
    }
}

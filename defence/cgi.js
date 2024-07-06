function setupClickCounter() {
    const clickButton = document.getElementById('click-button');
    const clickResult = document.getElementById('click-result');
    const resetButton = document.getElementById('reset-button');

    // 클릭 카운터 이벤트 리스너 설정
    if (clickButton && !clickButton.dataset.listenerAdded) {
        clickButton.addEventListener('click', function () {
            console.log('Clicking the button');
            fetch('/click_counter.py')
                .then(response => response.text())
                .then(data => {
                    clickResult.innerHTML = data;
                })
                .catch(error => console.error('Error:', error));
        });
        clickButton.dataset.listenerAdded = 'true';
        console.log('Click counter setup completed');
    }

    // 초기화 버튼 이벤트 리스너 설정
    if (resetButton && !resetButton.dataset.listenerAdded) {
        resetButton.addEventListener('click', function () {
            console.log('Resetting the counter');
            fetch('/reset_counter.py', { method: 'POST' })
                .then(response => response.text())
                .then(data => {
                    clickResult.innerHTML = '0';
                })
                .catch(error => console.error('Error:', error));
        });
        resetButton.dataset.listenerAdded = 'true';
        console.log('Reset counter setup completed');
    }
}

function setupCalculator() {
    const display = document.getElementById('calc-display');
    const buttons = document.querySelectorAll('.calculator button');
    const calculationResult = document.getElementById('calculation-result');
    let currentValue = '0';
    let operator = null;
    let firstValue = null;
    let secondValue = false;

    buttons.forEach(button => {
        button.addEventListener('click', () => {
            const value = button.innerText;

            if (value === 'AC') {
                currentValue = '0';
                firstValue = null;
                secondValue = false;
                operator = null;
            } else if (value === '±') {
                currentValue = (parseFloat(currentValue) * -1).toString();
            } else if (value === '%') {
                currentValue = (parseFloat(currentValue) / 100).toString();
            } else if (value === '÷' || value === '×' || value === '−' || value === '+') {
                if (firstValue === null) {
                    firstValue = parseFloat(currentValue);
                } else if (operator) {
                    const result = performCalculation(firstValue, parseFloat(currentValue), operator);
                    currentValue = result.toString();
                    firstValue = result;
                }
                operator = value;
                secondValue = true;
            } else if (value === '.') {
                if (!currentValue.includes('.')) {
                    currentValue += '.';
                }
            } else if (value === '=') {
                if (operator && firstValue !== null) {
                    const result = performCalculation(firstValue, parseFloat(currentValue), operator);
                    currentValue = result.toString();
                    firstValue = null;
                    operator = null;
                    secondValue = false;
                }
            } else {
                if (secondValue) {
                    currentValue = value;
                    secondValue = false;
                } else {
                    currentValue = currentValue === '0' ? value : currentValue + value;
                }
            }

            display.value = currentValue;
        });
    });

    function performCalculation(first, second, operator) {
        switch (operator) {
            case '÷':
                return first / second;
            case '×':
                return first * second;
            case '−':
                return first - second;
            case '+':
                return first + second;
        }
    }
}

// Immediately call the function to setup event listeners
setupClickCounter();
setupCalculator();

// Additionally, monitor changes to the DOM and re-setup the event listeners if needed
const observer = new MutationObserver((mutations) => {
    for (let mutation of mutations) {
        if (mutation.type === 'childList') {
            setupClickCounter();
            setupCalculator();
        }
    }
});

// Observe changes to the entire document body
observer.observe(document.body, { childList: true, subtree: true });

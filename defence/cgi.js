function setupClickCounter() {
    const clickButton = document.getElementById('click-button');
    const clickResult = document.getElementById('click-result');
    const resetButton = document.getElementById('reset-button');

    // 클릭 카운터 이벤트 리스너 설정
    if (clickButton && !clickButton.dataset.listenerAdded) {
        clickButton.addEventListener('click', function () {
            console.log('Clicking the button');
            fetch('/click_counter.py')
                .then(response => {
                    console.log('Response status:', response.status);
                    return response.text();
                })
                .then(data => {
                    clickResult.innerHTML = data;
                    console.log('Click counter response data:', data);
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
                .then(response => {
                    console.log('Response status:', response.status);
                    return response.text();
                })
                .then(data => {
                    clickResult.innerHTML = '0';
                    console.log('Reset counter response data:', data);
                })
                .catch(error => console.error('Error:', error));
        });
        resetButton.dataset.listenerAdded = 'true';
        console.log('Reset counter setup completed');
    }
}

document.addEventListener('DOMContentLoaded', (event) => {
    setupClickCounter();
});

function setupCalculator() {
    const display = document.getElementById('calc-display');
    const buttons = document.querySelectorAll('.calculator button');
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
                display.value = currentValue;
            } else if (value === '±') {
                currentValue = (parseFloat(currentValue) * -1).toString();
                display.value = currentValue;
            } else if (value === '%') {
                currentValue = (parseFloat(currentValue) / 100).toString();
                display.value = currentValue;
            } else if (value === '÷' || value === '×' || value === '−' || value === '+') {
                if (firstValue === null) {
                    firstValue = parseFloat(currentValue);
                } else if (operator) {
                    calculateResult();
                }
                operator = value;
                secondValue = true;
            } else if (value === '.') {
                if (!currentValue.includes('.')) {
                    currentValue += '.';
                    display.value = currentValue;
                }
            } else if (value === '=') {
                if (operator && firstValue !== null) {
                    calculateResult();
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
                display.value = currentValue;
            }
        });
    });

    function calculateResult() {
        const secondValue = parseFloat(currentValue);
        let operatorSymbol = operator;
        if (operatorSymbol === '÷') operatorSymbol = '/';
        if (operatorSymbol === '×') operatorSymbol = '*';
        if (operatorSymbol === '−') operatorSymbol = '-';

        const query = `first_value=${encodeURIComponent(firstValue)}&second_value=${encodeURIComponent(secondValue)}&operator=${encodeURIComponent(operatorSymbol)}`;

        fetch(`/calculator.py?${query}`)
            .then(response => {
                if (!response.ok) {
                    throw new Error("Network response was not ok");
                }
                return response.json();
            })
            .then(data => {
                if (data.error) {
                    console.error('Error:', data.error);
                    display.value = 'NaN';
                } else {
                    currentValue = data.result.toString();
                    display.value = currentValue;
                    firstValue = parseFloat(currentValue);
                }
            })
            .catch(error => {
                console.error('Error:', error);
                display.value = 'NaN';
            });
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

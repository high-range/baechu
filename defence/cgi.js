// Fetch existing feedback data on page load
function fetchNewFeedbacks() {
    fetch('/get_feedback.py')
        .then(response => {
            if (!response.ok) {
                throw new Error(`HTTP error! status: ${response.status}`);
            }
            return response.json();
        })
        .then(data => {
            console.log("Fetched feedback data:", data); // Debug message
            const feedbackList = document.getElementById('feedback-list');
            feedbackList.innerHTML = '';
            data.forEach(feedback => {
                const feedbackItem = document.createElement('li');
                feedbackItem.classList.add('feedback-item');
                feedbackItem.innerHTML = `
                    <p class="feedback-header">${feedback.name} | ${feedback.timestamp}</p>
                    <p>${feedback.message}</p>
                `;
                feedbackList.appendChild(feedbackItem);
            });
        })
        .catch(error => {
            console.error('Error:', error);
        });
}

// Setup feedback form submission
function setupFeedbackForm() {
    const feedbackButton = document.getElementById('submit-feedback');
    const feedbackResult = document.getElementById('feedback-result');

    if (feedbackButton) {
        feedbackButton.addEventListener('click', function () {
            const name = document.getElementById('name').value;
            const message = document.getElementById('message').value;

            if (name && message) {
                const formData = new URLSearchParams();
                formData.append('name', name);
                formData.append('message', message);

                fetch('/submit_feedback.py', {
                    method: 'POST',
                    body: formData
                })
                    .then(response => {
                        if (!response.ok) {
                            throw new Error(`HTTP error! status: ${response.status}`);
                        }
                        return response.json();
                    })
                    .then(data => {
                        const feedbackList = document.getElementById('feedback-list');
                        const feedbackItem = document.createElement('li');
                        feedbackItem.classList.add('feedback-item');
                        feedbackItem.innerHTML = `
                            <p class="feedback-header">${data.name} | ${data.timestamp}</p>
                            <p>${data.message}</p>
                        `;
                        feedbackList.appendChild(feedbackItem);

                        document.getElementById('name').value = '';
                        document.getElementById('message').value = '';
                        feedbackResult.textContent = 'Feedback submitted successfully!';
                    })
                    .catch(error => {
                        feedbackResult.textContent = 'Error submitting feedback.';
                        console.error('Error:', error);
                    });
            } else {
                feedbackResult.textContent = 'Please fill out both fields.';
            }
        });
    }
}

// 카운터 설정 함수
function setupClickCounter() {
    const clickButton = document.getElementById('click-button');
    const clickResult = document.getElementById('click-result');
    const resetButton = document.getElementById('reset-button');

    if (clickButton && !clickButton.dataset.listenerAdded) {
        clickButton.addEventListener('click', function () {
            fetch('/click_counter.py')
                .then(response => {
                    if (!response.ok) {
                        throw new Error(`HTTP error! status: ${response.status}`);
                    }
                    return response.text();
                })
                .then(data => {
                    clickResult.innerHTML = data;
                })
                .catch(error => console.error('Error:', error));
        });
        clickButton.dataset.listenerAdded = 'true';
    }

    if (resetButton && !resetButton.dataset.listenerAdded) {
        resetButton.addEventListener('click', function () {
            fetch('/reset_counter.py', { method: 'POST' })
                .then(response => {
                    if (!response.ok) {
                        throw new Error(`HTTP error! status: ${response.status}`);
                    }
                    return response.text();
                })
                .then(data => {
                    clickResult.innerHTML = '0';
                })
                .catch(error => console.error('Error:', error));
        });
        resetButton.dataset.listenerAdded = 'true';
    }
}

// 계산기 설정 함수
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
                    throw new Error(`HTTP error! status: ${response.status}`);
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

// 페이지 로드 시 초기 설정
fetchNewFeedbacks();
setupClickCounter();
setupCalculator();
setupFeedbackForm();
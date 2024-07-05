function handleRawPostRequest(event) {
    event.preventDefault();
    const form = document.getElementById('post-form-raw');
    const content = document.getElementById('raw-content').value;
    const contentType = document.getElementById('raw-content-type').value;
    fetch(form.action, {
        method: form.method,
        headers: { 'Content-Type': contentType },
        body: content
    })
    .then(response => response.text())
    .then(data => document.getElementById('post-result-raw').innerText = 'Raw POST request successful!')
    .catch(error => document.getElementById('post-result-raw').innerText = 'Raw POST request failed!');
}

function handleBinaryPostRequest(event) {
    event.preventDefault();
    const form = document.getElementById('post-form-binary');
    const formData = new FormData(form);
    fetch(form.action, {
        method: form.method,
        body: formData
    })
    .then(response => response.json())
    .then(data => document.getElementById('post-result-binary').innerHTML = 'Binary POST request successful! <a href="' + data.fileUrl + '">Download file</a>')
    .catch(error => document.getElementById('post-result-binary').innerText = 'Binary POST request failed!');
}

function copyCode(button) {
    const code = button.previousElementSibling.innerText.trim();
    navigator.clipboard.writeText(code).then(() => {
        showToast("Code copied to clipboard!");
    }).catch(err => {
        console.error('Failed to copy: ', err);
    });
}

function showToast(message) {
    const toast = document.createElement("div");
    toast.className = "toast";
    toast.innerText = message;
    document.body.appendChild(toast);
    setTimeout(() => { toast.className = "toast show"; }, 100);
    setTimeout(() => { toast.className = toast.className.replace("show", ""); document.body.removeChild(toast); }, 3000);
}

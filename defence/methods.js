function handleRawPostRequest(event) {
    event.preventDefault();
    const form = document.getElementById('post-form-raw');
    const content = document.getElementById('raw-content').value;
    const contentType = document.getElementById('raw-content-type').value;
    const filename = document.getElementById('raw-filename').value;

    const url = `${form.action}/${filename}`;

    fetch(url, {
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
    const fileInput = document.getElementById('binary-file');
    const file = fileInput.files[0];
    const filename = document.getElementById('binary-filename').value;

    if (file) {
        const reader = new FileReader();
        reader.onload = function (e) {
            const arrayBuffer = e.target.result;
            const url = `${form.action}/${filename}`;

            fetch(url, {
                method: form.method,
                headers: { 'Content-Type': 'application/octet-stream' },
                body: arrayBuffer
            })
                .then(response => response.text())
                .then(data => document.getElementById('post-result-binary').innerHTML = 'Binary POST request successful!')
                .catch(error => document.getElementById('post-result-binary').innerText = 'Binary POST request failed!');
        };
        reader.readAsArrayBuffer(file);
    } else {
        document.getElementById('post-result-binary').innerText = 'No file selected.';
    }
}

function sendDeleteRequest(url) {
    fetch(url, {
        method: 'DELETE'
    })
        .then(response => {
            return response.text().then(text => {
                const fullResponse = `Status: ${response.status} ${response.statusText}\n\n${text}`;
                document.getElementById('delete-result').innerText = fullResponse;
            });
        })
        .catch(error => {
            document.getElementById('delete-result').innerText = `Error: ${error.message}`;
        });
}

function handleFileUpload(event) {
    event.preventDefault();
    const form = document.getElementById('upload-form');
    const fileInput = document.getElementById('file');
    const file = fileInput.files[0];

    if (file) {
        const reader = new FileReader();
        reader.onload = function (e) {
            const arrayBuffer = e.target.result;
            const url = `${form.action}/${file.name}`;

            fetch(url, {
                method: form.method,
                headers: { 'Content-Type': 'application/octet-stream' },
                body: arrayBuffer
            })
                .then(response => response.text())
                .then(data => {
                    document.getElementById('upload-result').innerText = 'File uploaded successfully!';
                    fetchUploadedFiles(file.name);
                })
                .catch(error => {
                    document.getElementById('upload-result').innerText = 'File upload failed!';
                });
        };
        reader.readAsArrayBuffer(file);
    } else {
        document.getElementById('upload-result').innerText = 'No file selected.';
    }
}

function fetchUploadedFiles(filename) {
    const container = document.getElementById('uploaded-files-container');
    container.innerHTML = ''; // Clear the container first
    if (filename) {
        const fileElement = document.createElement('div');
        fileElement.className = 'file-item';
        fileElement.innerHTML = `<a href="/submit/${filename}" target="_blank">${filename}</a>`;
        container.appendChild(fileElement);
    } else {
        container.innerText = 'No files uploaded yet.';
    }
}

// Initial fetch to display any existing uploaded files on page load (optional)
// If you want to fetch and display all files initially, you can implement this function accordingly.
document.addEventListener('DOMContentLoaded', () => fetchUploadedFiles(null));


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

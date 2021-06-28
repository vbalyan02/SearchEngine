let socket = new WebSocket("ws://127.0.0.1:18909");

const  search_engine = {

    send_data :  () => {
        $("#result").html('');
        let query = document.getElementById("search").value;
        search_engine.waitForSocketConnection(socket, function() {
            console.log("message sent");
            socket.send(query);
        });

        socket.onmessage = function(e){
            search_engine.filter_data(e.data);
        };
    },

    filter_data : (data) => {
        let obj = JSON.parse(data);
        search_engine.fill_res_page(obj);
    },

    fill_res_page : (_data) => {
        console.log(_data.data[28].description);
        let info;
        for(let i = 0; i < _data.data.length; i++){
            info = _data.data[i];
            search_engine.create_site_block(info.url, info.title, info.description);
        }
    },

    create_site_block : (url, title, desc) => {
        let block = `<div class="site">
                    <a href="${url}"> ${title} </a>
                    <p class="url_desc">${desc}</p>
                </div>`;
        $("#result").append(block);
    },

    waitForSocketConnection : (socket, callback) => {
        setTimeout(
            function () {
                if (socket.readyState === 1) {
                    console.log("Connection is made")
                    if (callback != null){
                        callback();
                    }
                } else {
                    console.log("waiting for connection...")
                    waitForSocketConnection(socket, callback);
                }
            }, 100);
    },

}
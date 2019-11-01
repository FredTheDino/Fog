(() => {
    window.onload = () => {
        document.querySelectorAll(".hideable").forEach((o) => {
            o.onclick = function() {
                console.log();
                if (o.classList.contains("hide")) {
                    o.classList.remove("hide");
                } else {
                    o.classList.add("hide");
                }
            };
        });
    };
})();

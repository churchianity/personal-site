
// fortunately, chrome stores bookmarks as JSON and persists it to disk
// this file parses specifically my bookmarks for things I want to show on the site
const fs = require("fs");

const json = JSON.parse(fs.readFileSync("./bookmarks.json"))["roots"];

const bar = json["bookmark_bar"]["children"];
const ea = bar.find(bookmark => bookmark.name === "EA")["children"];
const programming = bar.find(bookmark => bookmark.name === "programming")["children"];
const philosophy = bar.find(bookmark => bookmark.name === "philosophy")["children"];
////////////////////

function parseBookmarks(bookmarks, toFilePath) {
    const out = [];
    for (let i = 0; i < bookmarks.length; i++) {
        const bm = bookmarks[i];
        if (bm && bm.type === "url") {
            out.push({
                name: bm.name,
                url: bm.url
            });
        }
    }
    fs.writeFileSync(toFilePath, out.map(bm => {
        return bm.name + "\n" + bm.url + "\n\n";
    }).join(''));
}

console.log(programming);

parseBookmarks(ea, "ea");
parseBookmarks(programming, "programming");
parseBookmarks(philosophy, "philosophy");

const other = json["other"]["children"];
parseBookmarks(other.find(bookmark => bookmark.name === "wikipedia")["children"], "wikipedia");



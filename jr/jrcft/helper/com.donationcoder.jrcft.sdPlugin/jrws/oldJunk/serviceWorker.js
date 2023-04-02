const staticDevCoffee = "dev-coffee-site-v1"
const assets = [
  "jrytControlr.html",
  "css/jryt.css",
  "js/jryt.js",
]

self.addEventListener("install", installEvent => {
  installEvent.waitUntil(
    caches.open(staticDevCoffee).then(cache => {
      cache.addAll(assets)
    })
  )
})
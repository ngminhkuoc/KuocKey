// Download buttons that follow the visitor's OS. No forms, no third-party scripts: the page sends nothing anywhere.
// window.VK_DL (written by build.py): {macos:{url}, windows:{url}, linux, ios, android}; null = no build yet.
(() => {
  const DL = window.VK_DL || {}, en = document.documentElement.lang === "en", pre = en ? "/en" : "";
  const NAME = { macos: "macOS", windows: "Windows", linux: "Linux", ios: "iOS", android: "Android" };
  const S = en
    ? { dl: os => `Download for ${os}`, store: "Download on the App Store", short: "Download", soon: "Soon" }
    : { dl: os => `Tải cho ${os}`, store: "Tải trên App Store", short: "Tải về", soon: "Sắp có" };

  const ua = navigator.userAgent;
  const os = /iPhone|iPad|iPod/.test(ua) || (/Macintosh/.test(ua) && navigator.maxTouchPoints > 1) ? "ios"
    : /Android/.test(ua) ? "android" : /Windows/.test(ua) ? "windows" : /Mac OS X|Macintosh/.test(ua) ? "macos"
    : /Linux|X11|CrOS/.test(ua) ? "linux" : null;
  const icon = p => `<svg class="ic" aria-hidden="true"><use href="/assets/os.svg#${p}"/></svg>`;

  // Buttons marked data-cta become "Download for <your OS>" (with "Soon" when that OS has no build yet).
  if (os) document.querySelectorAll("[data-cta]").forEach(a => {
    const d = DL[os], long = a.dataset.cta === "long";
    a.innerHTML = icon(os) + (long ? (os === "ios" && d ? S.store : S.dl(NAME[os])) : S.short) + (d ? "" : `<span class="soon">${S.soon}</span>`);
    a.href = long && d ? d.url : `${pre}/download#${os}`;   // the header button opens the download page
  });
  if (os) document.querySelectorAll(`.pcard[data-os="${os}"]`).forEach(c => c.classList.add("mine"));
})();

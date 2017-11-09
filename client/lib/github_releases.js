const fetch = require('node-fetch')

function getLatestGitHubRelease(repo, prefix, suffix) {
  let url
  if (process.env.USE_LOCAL_MOCK_GITHUB_SERVER) {
    url = `http://localhost:8080/repos/${repo}/releases/latest`
  } else {
    url = `https://api.github.com/repos/${repo}/releases/latest`
  }

  return new Promise((resolve, reject) => {
    fetch(url).then(response => {
      return response.json()
    }).then(release => {
      for (const asset of release.assets) {
        if (asset.name.startsWith(prefix) && asset.name.endsWith(suffix)) {
          resolve([release.tag_name, asset.browser_download_url])
        }
      }

      reject(new Error(`no matched GitHub release`))
    })
  })
}

module.exports = { getLatestGitHubRelease }

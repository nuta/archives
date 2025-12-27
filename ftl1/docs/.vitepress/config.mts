import { defineConfig } from 'vitepress'

export default defineConfig({
    title: 'FTL',
    description: 'An experimental microkernel-based general-purpose operating system',
    lang: 'en-US',
    cleanUrls: true,
    themeConfig: {
        sidebar: [
            {
                text: 'Getting Started',
                collapsed: false,
                items: [
                    { text: 'Quick Start', link: '/getting-started/quick-start' },
                    { text: 'Architecture', link: '/getting-started/architecture' },
                    { text: 'Comparisons', link: '/getting-started/comparisons' },
                    { text: 'Why Rust?', link: '/getting-started/why-rust' }
                ]
            },
            {
                text: 'Learn',
                collapsed: false,
                items: [
                    { text: 'Process', link: '/learn/process' },
                    { text: 'Isolation', link: '/learn/isolation' },
                    { text: 'Channel', link: '/learn/channel' },
                    { text: 'Environ', link: '/learn/environ' },
                    { text: 'Linux Compatibility', link: '/learn/linux-compatibility' }
                ]
            },
            {
                text: 'Guides',
                collapsed: false,
                items: [
                    { text: 'Writing Applications', link: '/guides/writing-applications' },
                    { text: 'Writing Device Drivers', link: '/guides/writing-device-drivers' },
                    { text: 'Debugging', link: '/guides/debugging' },
                    { text: 'Testing', link: '/guides/testing' },
                    { text: 'Deploying', link: '/guides/deploying' }
                ]
            },
            {
                text: 'Changes',
                collapsed: false,
                items: [
                    { text: 'Upcoming Features', link: '/changes/upcoming-features' }
                ]
            },
            {
                text: 'Advanced',
                collapsed: false,
                items: [
                    { text: 'Kernel Development', link: '/advanced/kernel-development' }
                ]
            },
            {
                'text': 'Links',
                items: [
                    { text: 'GitHub', link: 'https://github.com/ftl-os/ftl' }
                ]
            }
        ],

        socialLinks: [
            { icon: 'github', link: 'https://github.com/ftl-os/ftl' }
        ],

        search: {
            provider: 'local'
        }
    }
})

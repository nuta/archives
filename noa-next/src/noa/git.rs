use anyhow::{Context, Result};
use git2::DiffOptions;
use libgit2_sys::{
    git_blob, git_blob_free, git_diff_blob_to_buffer, git_diff_delta, git_diff_hunk,
    git_libgit2_init, git_object_peel, git_repository_open, git_revparse_single, GIT_OBJECT_BLOB,
    GIT_OK,
};

use std::{
    ffi::{c_void, CString},
    ops,
    os::raw::{c_char, c_int},
    path::{Path, PathBuf},
    ptr,
};

#[derive(Debug, Clone, Copy, PartialEq)]
pub enum DiffType {
    Added,
    Modified,
    Removed,
}

#[derive(Debug, Clone)]
pub struct LineRangeDiff {
    pub diff_type: DiffType,
    pub range: ops::Range<usize>,
}

struct DiffCallbackContext {
    diffs: Vec<LineRangeDiff>,
}

macro_rules! try_libgit_func {
    ($summary:expr, $expr:expr) => {{
        let ret = $expr;
        if ret < 0 {
            let error = ::libgit2_sys::git_error_last();
            if error.is_null() {
                ::anyhow::bail!("libgit: failed to {}: returned {}", $summary, ret);
            }

            let message = ::std::ffi::CStr::from_ptr((*error).message as *const _)
                .to_str()
                .unwrap();

            ::anyhow::bail!(format!("libgit: failed to {}: {}", $summary, message));
        }
    }};
}

extern "C" fn diff_callback(
    _delta: *const git_diff_delta,
    hunk: *const git_diff_hunk,
    ctx: *mut c_void,
) -> c_int {
    unsafe {
        let ctx = &mut *(ctx as *mut DiffCallbackContext);
        let hunk = &*(hunk);

        // Resolve hunks into LineRanegDiff. Ported from the Atom editor.
        let start_y = hunk.new_start - 1;
        let end_y = hunk.new_start + hunk.new_lines - 1;
        let (range, diff_type) = if hunk.old_lines == 0 && hunk.new_lines > 0 {
            ((start_y as usize)..(end_y as usize), DiffType::Added)
        } else if hunk.new_lines == 0 && hunk.old_lines > 0 {
            if start_y < 0 {
                (0..0, DiffType::Removed)
            } else {
                ((start_y as usize)..(end_y as usize), DiffType::Removed)
            }
        } else {
            ((start_y as usize)..(end_y as usize), DiffType::Modified)
        };

        ctx.diffs.push(LineRangeDiff { diff_type, range });
    }

    GIT_OK
}

pub fn resolve_git_dir(dir: &Path) -> Option<PathBuf> {
    for dir in dir.ancestors() {
        if dir.join(".git").exists() {
            return Some(dir.to_path_buf());
        }
    }

    None
}

#[derive(Clone)]
pub struct Repo {
    repo_dir: PathBuf,
}

impl Repo {
    pub fn open(dir: &Path) -> Result<Repo> {
        let repo_dir = resolve_git_dir(dir)
            .context("not in a git repository")?
            .canonicalize()?;
        Ok(Repo { repo_dir })
    }

    pub fn compute_line_diffs(&self, path: &Path, text: &str) -> Result<Vec<LineRangeDiff>> {
        debug_assert!(self.repo_dir.is_absolute());
        debug_assert!(path.is_absolute());

        let mut ctx = DiffCallbackContext { diffs: Vec::new() };
        unsafe {
            let spec = CString::new(format!(
                "HEAD:{}",
                path.strip_prefix(&self.repo_dir)?.display()
            ))
            .unwrap();
            let repo_dir = CString::new(self.repo_dir.as_os_str().to_str().unwrap()).unwrap();

            let mut opts = DiffOptions::default();
            opts.context_lines(0);

            try_libgit_func!("init libgit2", git_libgit2_init());

            let mut repo = ptr::null_mut();
            try_libgit_func!(
                "open the repo",
                git_repository_open(&mut repo, repo_dir.as_ptr())
            );

            let mut obj = ptr::null_mut();
            try_libgit_func!(
                "open the blob",
                git_revparse_single(&mut obj, repo, spec.as_ptr())
            );
            let mut blob = ptr::null_mut();
            try_libgit_func!(
                "peel the object",
                git_object_peel(&mut blob, obj, GIT_OBJECT_BLOB)
            );

            try_libgit_func!(
                "compute diff",
                git_diff_blob_to_buffer(
                    /* old_blob */ blob as *const git_blob,
                    /* old_as_path */ ptr::null(),
                    /* buffer */ text.as_bytes().as_ptr() as *const c_char,
                    /* buffer_len */ text.as_bytes().len(),
                    /* buffer_as_path */ ptr::null(),
                    /* options */ opts.raw(),
                    /* file_cb */ None,
                    /* binary_cb */ None,
                    /* hunk_cb */ Some(diff_callback),
                    /* line_cb */ None,
                    /* payload */ &mut ctx as *mut _ as *mut c_void,
                )
            );

            git_blob_free(blob as *mut git_blob);
        }

        Ok(ctx.diffs)
    }
}

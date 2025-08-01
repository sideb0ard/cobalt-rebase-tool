#!/usr/bin/env python3
# Copyright 2021 The Chromium Authors
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
"""This script is used to fetch reclient cfgs.

This is renamed to configure_reclient_cfgs.py, delete this file if there are no
reference to this file.
"""

import sys

<<<<<<< HEAD
import configure_reclient_cfgs
=======
def NaclRevision():
    nacl_dir = os.path.join(CHROMIUM_SRC, 'native_client')
    if not os.path.exists(nacl_dir):
      return None
    return subprocess.check_output(
        ['git', 'log', '-1', '--format=%H'],
        cwd= nacl_dir,
    ).decode('utf-8').strip()

class CipdError(Exception):
  """Raised by fetch_reclient_cfgs on fatal cipd error."""

def CipdEnsure(pkg_name, ref, directory, quiet):
    logging.info('ensure %s %s in %s' % (pkg_name, ref, directory))
    log_level = 'warning' if quiet else 'debug'
    ensure_file = """
$ParanoidMode CheckPresence
{pkg} {ref}
""".format(pkg=pkg_name, ref=ref)
    try:
      output = subprocess.check_output(
          ' '.join(['cipd', 'ensure', '-log-level=' + log_level,
                    '-root', directory, '-ensure-file', '-']),
          shell=True, input=ensure_file, stderr=subprocess.STDOUT,
          universal_newlines=True)
      logging.info(output)
    except subprocess.CalledProcessError as e:
      raise CipdError(e.output) from e


def RbeProjectFromInstance(instance):
    m = re.fullmatch(r'projects/([-\w]+)/instances/[-\w]+', instance)
    if not m:
        return None
    return m.group(1)

def GenerateReproxyCfg(reproxy_cfg_template, rbe_instance):
    tmpl_path = os.path.join(
        THIS_DIR, 'reproxy_cfg_templates', reproxy_cfg_template)
    logging.info(f'generate reproxy.cfg using {tmpl_path}')
    if not os.path.isfile(tmpl_path):
        logging.warning(f"{tmpl_path} does not exist")
        return False
    with open(tmpl_path) as f:
      reproxy_cfg_tmpl = string.Template(REPROXY_CFG_HEADER+f.read())
    scandeps_bin_name = 'scandeps_server'
    if sys.platform.startswith('win'):
       scandeps_bin_name += ".exe"
    reproxy_cfg = reproxy_cfg_tmpl.substitute({
      'rbe_instance': rbe_instance,
      'reproxy_cfg_template': reproxy_cfg_template,
      'scandeps_bin_path':
        os.path.join(CHROMIUM_SRC, 'buildtools', 'reclient', scandeps_bin_name),
    })
    reproxy_cfg_path = os.path.join(THIS_DIR, 'reproxy.cfg')
    with open(reproxy_cfg_path, 'w') as f:
      f.write(reproxy_cfg)
    return True

def main():
    parser = argparse.ArgumentParser(
       description='fetch reclient cfgs',
       formatter_class=argparse.RawTextHelpFormatter)
    parser.add_argument(
        '--rewrapper_cfg_project', '--rbe_project',
        help='RBE instance project id for rewrapper configs. '
             'Only specify if different from --rbe_instance\n'
             'TODO(b/270201776) --rbe_project is deprecated, '
             'remove once no more usages exist')
    parser.add_argument(
        '--reproxy_cfg_template',
        help='If set REPROXY_CFG_TEMPLATE will be used to generate '
             'reproxy.cfg. --rbe_instance must be specified.')
    parser.add_argument('--rbe_instance',
                        help='RBE instance for rewrapper and reproxy configs',
                        default=os.environ.get('RBE_instance'))
    parser.add_argument('--cipd_prefix',
                        help='cipd package name prefix',
                        default='infra_internal/rbe/reclient_cfgs')
    parser.add_argument(
        '--quiet',
        help='Suppresses info logs',
        action='store_true')
    parser.add_argument(
        '--hook',
        help='Indicates that this script was run as a gclient hook',
        action='store_true')

    args = parser.parse_args()

    logging.basicConfig(level=logging.WARNING if args.quiet else logging.INFO,
                        format="%(message)s")


    if args.reproxy_cfg_template:
        if not args.rbe_instance:
            logging.error(
              '--rbe_instance is required if --reproxy_cfg_template is set')
            return 1
        if not GenerateReproxyCfg(args.reproxy_cfg_template, args.rbe_instance):
           return 1
# COBALT Disables CIPD fetch of rewrapper configs in favor of local rewrapper configs.
        return 0
    return 1
# COBALT END

    if not args.rewrapper_cfg_project and not args.rbe_instance:
        logging.error(
           'At least one of --rbe_instance and --rewrapper_cfg_project '
           'must be provided')
        return 1

    rbe_project = args.rewrapper_cfg_project
    if not args.rewrapper_cfg_project:
        rbe_project = RbeProjectFromInstance(args.rbe_instance)

    logging.info('fetch reclient_cfgs for RBE project %s...' % rbe_project)

    cipd_prefix = posixpath.join(args.cipd_prefix, rbe_project)

    tool_revisions = {
        'chromium-browser-clang': ClangRevision(),
        'nacl': NaclRevision(),
        'python': '3.8.0',
    }
    for toolchain in tool_revisions:
      revision = tool_revisions[toolchain]
      if not revision:
        logging.info('failed to detect %s revision' % toolchain)
        continue

      toolchain_root = os.path.join(THIS_DIR, toolchain)
      cipd_ref = 'revision/' + revision
      # 'cipd ensure' initializes the directory.
      try:
        CipdEnsure(posixpath.join(cipd_prefix, toolchain),
                    ref=cipd_ref,
                    directory=toolchain_root,
                    quiet=args.quiet)
      except CipdError as e:
        logging.error(e)
        if args.hook:
          logging.error(
             "Developer builds are not supported yet. "
             "Remove '\"download_remoteexec_cfg\":True' from .gclient")
        return 1
      # support legacy (win-cross-experiments) and new (win-cross)
      # TODO(crbug.com/1407557): drop -experiments support
      wcedir = os.path.join(THIS_DIR, 'win-cross', toolchain)
      if not os.path.exists(wcedir):
          os.makedirs(wcedir, mode=0o755)
      for win_cross_cfg_dir in ['win-cross','win-cross-experiments']:
          if os.path.exists(os.path.join(toolchain_root, win_cross_cfg_dir)):
              # copy in win-cross*/toolchain
              # as windows may not use symlinks.
              for cfg in glob.glob(os.path.join(toolchain_root,
                                                win_cross_cfg_dir,
                                                '*.cfg')):
                  fname = os.path.join(wcedir, os.path.basename(cfg))
                  if os.path.exists(fname):
                    os.chmod(fname, 0o777)
                    os.remove(fname)
                  logging.info('Copy from %s to %s...' % (cfg, fname))
                  shutil.copy(cfg, fname)

    return 0
>>>>>>> e7fdfcb7550 (Set up Cobalt build acceleration with RBE (#4604))

if __name__ == '__main__':
    sys.exit(configure_reclient_cfgs.main())

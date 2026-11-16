# Security Policy

## Supported versions

Only the latest release is supported. Fixes go to master and then into a new release.

## Reporting a vulnerability

1. Share the details of the vulnerability privately by emailing
   *officesecurity@lists.freedesktop.org*. That list is the security team for LibreOffice,
   Collabora Online and the document libraries they use, libeot among them.
2. We acknowledge your report and then verify the vulnerability.
3. We disclose the vulnerability to the public after a release with the fix, as an advisory with
   a CVE ID at https://github.com/umanwizard/libeot/security/advisories.
4. We credit reporters in the advisory, but reporters may remain anonymous if they wish.

A report is most useful with the font file that triggers the problem, the version or commit you
tested, and the sanitizer output if you have it.

## What we do not treat as a vulnerability

We fix these as ordinary bugs, without a CVE:

- Denial of service on its own, including crashes from a NULL pointer, hangs and resource
  exhaustion. If it leads to something more, such as code execution or data disclosure, we assess
  that instead.
- Missing hardening with no demonstrated exploit.
- A finding against code that is in no release. We fix it on master without an advisory.

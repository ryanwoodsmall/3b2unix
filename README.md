# at&t 3b2 unix

this is at&t 3b2 unix source code

source:

- https://loomcom.com/3b2/source_code_provision.html

extracted with dd and heirloom cpio:

```
dd if=image.tap bs=512 skip=31 conv=notrunc | cpio -dvmui -H odc
```

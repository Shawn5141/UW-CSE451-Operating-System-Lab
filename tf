kernel/fs.c:  memmove(sb, bp->data, sizeof(*sb));
kernel/fs.c:// not stored on disk: ip->ref and ip->flags.
kernel/fs.c:    if (ip->ref > 0 && ip->dev == dev && ip->inum == inum) {
kernel/fs.c:      ip->ref++;
kernel/fs.c:    if (empty == 0 && ip->ref == 0) // Remember empty slot.
kernel/fs.c:  ip->ref = 1;
kernel/fs.c:  ip->valid = 0;
kernel/fs.c:  ip->dev = dev;
kernel/fs.c:  ip->inum = inum;
kernel/fs.c:  ip->ref++;
kernel/fs.c:  if (ip->ref == 1)
kernel/fs.c:    ip->type = 0;
kernel/fs.c:  ip->ref--;
kernel/fs.c:  if(ip == 0 || ip->ref < 1)
kernel/fs.c:  acquiresleep(&ip->lock);
kernel/fs.c:  if (ip->valid == 0) {
kernel/fs.c:    read_dinode(ip->inum, &dip);
kernel/fs.c:    ip->type = dip.type;
kernel/fs.c:    ip->devid = dip.devid;
kernel/fs.c:    ip->size = dip.size;
kernel/fs.c:    ip->data = dip.data;
kernel/fs.c:    ip->valid = 1;
kernel/fs.c:    if (ip->type == 0)
kernel/fs.c:  if(ip == 0 || !holdingsleep(&ip->lock) || ip->ref < 1)
kernel/fs.c:  releasesleep(&ip->lock);
kernel/fs.c:// Caller must hold ip->lock.
kernel/fs.c:  if (!holdingsleep(&ip->lock))
kernel/fs.c:  st->dev = ip->dev;
kernel/fs.c:  st->ino = ip->inum;
kernel/fs.c:  st->type = ip->type;
kernel/fs.c:  st->size = ip->size;
kernel/fs.c:// Caller must hold ip->lock.
kernel/fs.c:  if (!holdingsleep(&ip->lock))
kernel/fs.c:  if (ip->type == T_DEV) {
kernel/fs.c:    if (ip->devid < 0 || ip->devid >= NDEV || !devsw[ip->devid].read)
kernel/fs.c:    return devsw[ip->devid].read(ip, dst, n);
kernel/fs.c:  if (off > ip->size || off + n < off)
kernel/fs.c:  if (off + n > ip->size)
kernel/fs.c:    n = ip->size - off;
kernel/fs.c:    bp = bread(ip->dev, ip->data.startblkno + off / BSIZE);
kernel/fs.c:    memmove(dst, bp->data + off % BSIZE, m);
kernel/fs.c:// Caller must hold ip->lock.
kernel/fs.c:  if (!holdingsleep(&ip->lock))
kernel/fs.c:  if (ip->type == T_DEV) {
kernel/fs.c:    if (ip->devid < 0 || ip->devid >= NDEV || !devsw[ip->devid].write)
kernel/fs.c:    return devsw[ip->devid].write(ip, src, n);
kernel/fs.c:  if (dp->type != T_DIR)
kernel/fs.c:  for (off = 0; off < dp->size; off += sizeof(de)) {
kernel/fs.c:      return iget(dp->dev, inum);
kernel/fs.c:    if (ip->type != T_DIR) {
Binary file kernel/.file.c.swp matches
kernel/mp.c:  if ((mp = mpsearch()) == 0 || mp->physaddr == 0)
kernel/mp.c:  conf = (struct mpconf *)P2V((uint64_t)mp->physaddr);
kernel/mp.c:  if (mp->imcrp) {
kernel/proc.c:    if (p->state == UNUSED)
kernel/proc.c:  p->state = EMBRYO;
kernel/proc.c:  p->pid = nextpid++;
kernel/proc.c:  p->killed = 0;
kernel/proc.c:  if ((p->kstack = kalloc()) == 0) {
kernel/proc.c:    p->state = UNUSED;
kernel/proc.c:  sp = p->kstack + KSTACKSIZE;
kernel/proc.c:  sp -= sizeof *p->tf;
kernel/proc.c:  p->tf = (struct trap_frame *)sp;
kernel/proc.c:  sp -= sizeof *p->context;
kernel/proc.c:  p->context = (struct context *)sp;
kernel/proc.c:  memset(p->context, 0, sizeof *p->context);
kernel/proc.c:  p->context->rip = (uint64_t)forkret;
kernel/proc.c:  assertm(vspaceinit(&p->vspace) == 0, "error initializing process's virtual address descriptor");
kernel/proc.c:  vspaceinitcode(&p->vspace, _binary_out_initcode_start, (int64_t)_binary_out_initcode_size);
kernel/proc.c:  memset(p->tf, 0, sizeof(*p->tf));
kernel/proc.c:  p->tf->cs = (SEG_UCODE << 3) | DPL_USER;
kernel/proc.c:  p->tf->ss = (SEG_UDATA << 3) | DPL_USER;
kernel/proc.c:  p->tf->rflags = FLAGS_IF;
kernel/proc.c:  p->tf->rip = VRBOT(&p->vspace.regions[VR_CODE]);  // beginning of initcode.S
kernel/proc.c:  p->tf->rsp = VRTOP(&p->vspace.regions[VR_USTACK]);
kernel/proc.c:  safestrcpy(p->name, "initcode", sizeof(p->name));
kernel/proc.c:  // this assignment to p->state lets other cores
kernel/proc.c:  p->state = RUNNABLE;
kernel/proc.c:   vspacecopy(&myproc()->vspace,&p->vspace);
kernel/proc.c:   p->tf->cs = myproc()->tf->cs;
kernel/proc.c:   p->tf->ss = myproc()->tf->ss;
kernel/proc.c:   p->tf->rflags = myproc()->tf->rlfags;
kernel/proc.c:   p->tf->rip = myproc()->tf->rip;  // beginning of initcode.S
kernel/proc.c:   p->tf->rsp = myproc()->tf->rsp;
kernel/proc.c:      if (p->state != RUNNABLE)
kernel/proc.c:      p->state = RUNNING;
kernel/proc.c:      swtch(&mycpu()->scheduler, p->context);
kernel/proc.c:      // It should have changed its p->state before coming back.
kernel/proc.c:  // change p->state and then call sched.
kernel/proc.c:    if (p->state == SLEEPING && p->chan == chan)
kernel/proc.c:      p->state = RUNNABLE;
kernel/proc.c:    if (p->pid == pid) {
kernel/proc.c:      p->killed = 1;
kernel/proc.c:      if (p->state == SLEEPING)
kernel/proc.c:        p->state = RUNNABLE;
kernel/proc.c:    if (p->state == UNUSED)
kernel/proc.c:    if (p->state != 0 && p->state < NELEM(states) && states[p->state])
kernel/proc.c:      state = states[p->state];
kernel/proc.c:    cprintf("%d %s %s", p->pid, state, p->name);
kernel/proc.c:    if (p->state == SLEEPING) {
kernel/proc.c:      getcallerpcs((uint64_t *)p->context->rbp, pc);
kernel/proc.c:    if (p->pid == pid)
kernel/vspace.c:  if (!p->kstack)
kernel/vspace.c:  if (!p->vspace.pgtbl)
kernel/vspace.c:  mycpu()->ts.rsp0 = (uint64_t)p->kstack + KSTACKSIZE;
kernel/vspace.c:  lcr3(V2P(p->vspace.pgtbl));
kernel/file.c:    if(p->pftable[pfd]==NULL) { //TODO Not sure how to check is emtpty
kernel/file.c:      p->pftable[pfd] = &ftable[gfd];
kernel/file.c:  if(p->pftable[fd] == NULL)
kernel/file.c:  struct file_info f = *(p->pftable[fd]);
kernel/file.c:   if(p->pftable[fd]==NULL)return -1;
kernel/file.c:   struct file_info f=*(p->pftable[fd]);
kernel/file.c:     p->pftable[fd]->ref -= 1;
kernel/file.c:     p->pftable[fd]->iptr = 0;
kernel/file.c:     p->pftable[fd]->ref = 0;
kernel/file.c:     p->pftable[fd]->path = 0;
kernel/file.c:     p->pftable[fd]->access_permission = 0;
kernel/file.c:     p->pftable[fd]->offset=0;
kernel/file.c:   //   cprintf("%s close  for fd =%d and ref is %d \n",f.path,fd,p->pftable[fd]->ref);
kernel/file.c:   p->pftable[fd] = NULL;
kernel/file.c:   if(p->pftable[fd]==NULL)return -1;
kernel/file.c:   struct file_info f=*(p->pftable[fd]);
kernel/file.c:   p->pftable[fd]->offset+=offset;
kernel/file.c:   //cprintf("offset right now %d and try to read %d bytes and got %d read \n",p->pftable[fd]->offset,bytes_read,offset);
kernel/file.c:   if(p->pftable[fd]==NULL)return -1;
kernel/file.c:   struct file_info f=*(p->pftable[fd]);
kernel/file.c:   if(p->pftable[fd]==NULL)return -1;
kernel/file.c:   struct file_info f=*(p->pftable[fd]);
kernel/file.c:    if(p->pftable[i] == NULL) {
kernel/file.c:      p->pftable[i] = p->pftable[fd]; 
kernel/file.c:      p->pftable[fd]->ref++;     //increase reference count
kernel/file.c:      //cprintf("dup file:%s from fd = %d to new fd = %d and ref become %d \n",f.path,fd,i,p->pftable[fd]->ref);
kernel/z:fs.c:  memmove(sb, bp->data, sizeof(*sb));
kernel/z:fs.c:// not stored on disk: ip->ref and ip->flags.
kernel/z:fs.c:    if (ip->ref > 0 && ip->dev == dev && ip->inum == inum) {
kernel/z:fs.c:      ip->ref++;
kernel/z:fs.c:    if (empty == 0 && ip->ref == 0) // Remember empty slot.
kernel/z:fs.c:  ip->ref = 1;
kernel/z:fs.c:  ip->valid = 0;
kernel/z:fs.c:  ip->dev = dev;
kernel/z:fs.c:  ip->inum = inum;
kernel/z:fs.c:  ip->ref++;
kernel/z:fs.c:  if (ip->ref == 1)
kernel/z:fs.c:    ip->type = 0;
kernel/z:fs.c:  ip->ref--;
kernel/z:fs.c:  if(ip == 0 || ip->ref < 1)
kernel/z:fs.c:  acquiresleep(&ip->lock);
kernel/z:fs.c:  if (ip->valid == 0) {
kernel/z:fs.c:    read_dinode(ip->inum, &dip);
kernel/z:fs.c:    ip->type = dip.type;
kernel/z:fs.c:    ip->devid = dip.devid;
kernel/z:fs.c:    ip->size = dip.size;
kernel/z:fs.c:    ip->data = dip.data;
kernel/z:fs.c:    ip->valid = 1;
kernel/z:fs.c:    if (ip->type == 0)
kernel/z:fs.c:  if(ip == 0 || !holdingsleep(&ip->lock) || ip->ref < 1)
kernel/z:fs.c:  releasesleep(&ip->lock);
kernel/z:fs.c:// Caller must hold ip->lock.
kernel/z:fs.c:  if (!holdingsleep(&ip->lock))
kernel/z:fs.c:  st->dev = ip->dev;
kernel/z:fs.c:  st->ino = ip->inum;
kernel/z:fs.c:  st->type = ip->type;
kernel/z:fs.c:  st->size = ip->size;
kernel/z:fs.c:// Caller must hold ip->lock.
kernel/z:fs.c:  if (!holdingsleep(&ip->lock))
kernel/z:fs.c:  if (ip->type == T_DEV) {
kernel/z:fs.c:    if (ip->devid < 0 || ip->devid >= NDEV || !devsw[ip->devid].read)
kernel/z:fs.c:    return devsw[ip->devid].read(ip, dst, n);
kernel/z:fs.c:  if (off > ip->size || off + n < off)
kernel/z:fs.c:  if (off + n > ip->size)
kernel/z:fs.c:    n = ip->size - off;
kernel/z:fs.c:    bp = bread(ip->dev, ip->data.startblkno + off / BSIZE);
kernel/z:fs.c:    memmove(dst, bp->data + off % BSIZE, m);
kernel/z:fs.c:// Caller must hold ip->lock.
kernel/z:fs.c:  if (!holdingsleep(&ip->lock))
kernel/z:fs.c:  if (ip->type == T_DEV) {
kernel/z:fs.c:    if (ip->devid < 0 || ip->devid >= NDEV || !devsw[ip->devid].write)
kernel/z:fs.c:    return devsw[ip->devid].write(ip, src, n);
kernel/z:fs.c:  if (dp->type != T_DIR)
kernel/z:fs.c:  for (off = 0; off < dp->size; off += sizeof(de)) {
kernel/z:fs.c:      return iget(dp->dev, inum);
kernel/z:fs.c:    if (ip->type != T_DIR) {
kernel/z:mp.c:  if ((mp = mpsearch()) == 0 || mp->physaddr == 0)
kernel/z:mp.c:  conf = (struct mpconf *)P2V((uint64_t)mp->physaddr);
kernel/z:mp.c:  if (mp->imcrp) {
kernel/z:proc.c:    if (p->state == UNUSED)
kernel/z:proc.c:  p->state = EMBRYO;
kernel/z:proc.c:  p->pid = nextpid++;
kernel/z:proc.c:  p->killed = 0;
kernel/z:proc.c:  if ((p->kstack = kalloc()) == 0) {
kernel/z:proc.c:    p->state = UNUSED;
kernel/z:proc.c:  sp = p->kstack + KSTACKSIZE;
kernel/z:proc.c:  sp -= sizeof *p->tf;
kernel/z:proc.c:  p->tf = (struct trap_frame *)sp;
kernel/z:proc.c:  sp -= sizeof *p->context;
kernel/z:proc.c:  p->context = (struct context *)sp;
kernel/z:proc.c:  memset(p->context, 0, sizeof *p->context);
kernel/z:proc.c:  p->context->rip = (uint64_t)forkret;
kernel/z:proc.c:  assertm(vspaceinit(&p->vspace) == 0, "error initializing process's virtual address descriptor");
kernel/z:proc.c:  vspaceinitcode(&p->vspace, _binary_out_initcode_start, (int64_t)_binary_out_initcode_size);
kernel/z:proc.c:  memset(p->tf, 0, sizeof(*p->tf));
kernel/z:proc.c:  p->tf->cs = (SEG_UCODE << 3) | DPL_USER;
kernel/z:proc.c:  p->tf->ss = (SEG_UDATA << 3) | DPL_USER;
kernel/z:proc.c:  p->tf->rflags = FLAGS_IF;
kernel/z:proc.c:  p->tf->rip = VRBOT(&p->vspace.regions[VR_CODE]);  // beginning of initcode.S
kernel/z:proc.c:  p->tf->rsp = VRTOP(&p->vspace.regions[VR_USTACK]);
kernel/z:proc.c:  safestrcpy(p->name, "initcode", sizeof(p->name));
kernel/z:proc.c:  // this assignment to p->state lets other cores
kernel/z:proc.c:  p->state = RUNNABLE;
kernel/z:proc.c:      if (p->state != RUNNABLE)
kernel/z:proc.c:      p->state = RUNNING;
kernel/z:proc.c:      swtch(&mycpu()->scheduler, p->context);
kernel/z:proc.c:      // It should have changed its p->state before coming back.
kernel/z:proc.c:  // change p->state and then call sched.
kernel/z:proc.c:    if (p->state == SLEEPING && p->chan == chan)
kernel/z:proc.c:      p->state = RUNNABLE;
kernel/z:proc.c:    if (p->pid == pid) {
kernel/z:proc.c:      p->killed = 1;
kernel/z:proc.c:      if (p->state == SLEEPING)
kernel/z:proc.c:        p->state = RUNNABLE;
kernel/z:proc.c:    if (p->state == UNUSED)
kernel/z:proc.c:    if (p->state != 0 && p->state < NELEM(states) && states[p->state])
kernel/z:proc.c:      state = states[p->state];
kernel/z:proc.c:    cprintf("%d %s %s", p->pid, state, p->name);
kernel/z:proc.c:    if (p->state == SLEEPING) {
kernel/z:proc.c:      getcallerpcs((uint64_t *)p->context->rbp, pc);
kernel/z:proc.c:    if (p->pid == pid)
kernel/z:vspace.c:  if (!p->kstack)
kernel/z:vspace.c:  if (!p->vspace.pgtbl)
kernel/z:vspace.c:  mycpu()->ts.rsp0 = (uint64_t)p->kstack + KSTACKSIZE;
kernel/z:vspace.c:  lcr3(V2P(p->vspace.pgtbl));
user/umalloc.c:  for (p = freep; !(bp > p && bp < p->s.ptr); p = p->s.ptr)
user/umalloc.c:    if (p >= p->s.ptr && (bp > p || bp < p->s.ptr))
user/umalloc.c:  if (bp + bp->s.size == p->s.ptr) {
user/umalloc.c:    bp->s.size += p->s.ptr->s.size;
user/umalloc.c:    bp->s.ptr = p->s.ptr->s.ptr;
user/umalloc.c:    bp->s.ptr = p->s.ptr;
user/umalloc.c:  if (p + p->s.size == bp) {
user/umalloc.c:    p->s.size += bp->s.size;
user/umalloc.c:    p->s.ptr = bp->s.ptr;
user/umalloc.c:    p->s.ptr = bp;
user/umalloc.c:  hp->s.size = nu;
user/umalloc.c:  for (p = prevp->s.ptr;; prevp = p, p = p->s.ptr) {
user/umalloc.c:    if (p->s.size >= nunits) {
user/umalloc.c:      if (p->s.size == nunits)
user/umalloc.c:        prevp->s.ptr = p->s.ptr;
user/umalloc.c:        p->s.size -= nunits;
user/umalloc.c:        p += p->s.size;
user/umalloc.c:        p->s.size = nunits;
user/ls.c:  for (p = path + strlen(path); p >= path && *p != '/'; p--)
out/initcode.asm:  for (p = freep; !(bp > p && bp < p->s.ptr); p = p->s.ptr)
out/initcode.asm:    if (p >= p->s.ptr && (bp > p || bp < p->s.ptr))
out/initcode.asm:  for (p = freep; !(bp > p && bp < p->s.ptr); p = p->s.ptr)
out/initcode.asm:  if (bp + bp->s.size == p->s.ptr) {
out/initcode.asm:    bp->s.size += p->s.ptr->s.size;
out/initcode.asm:    bp->s.ptr = p->s.ptr->s.ptr;
out/initcode.asm:    bp->s.ptr = p->s.ptr;
out/initcode.asm:  if (p + p->s.size == bp) {
out/initcode.asm:    p->s.size += bp->s.size;
out/initcode.asm:    p->s.ptr = bp->s.ptr;
out/initcode.asm:    p->s.ptr = bp;
out/initcode.asm:  hp->s.size = nu;
out/initcode.asm:  for (p = prevp->s.ptr;; prevp = p, p = p->s.ptr) {
out/initcode.asm:    if (p->s.size >= nunits) {
out/initcode.asm:      if (p->s.size == nunits)
out/initcode.asm:        prevp->s.ptr = p->s.ptr;
out/initcode.asm:        p->s.size -= nunits;
out/initcode.asm:        p += p->s.size;
out/initcode.asm:        p->s.size = nunits;
out/initcode.asm:  for (p = prevp->s.ptr;; prevp = p, p = p->s.ptr) {
out/initcode.asm:    if (p->s.size >= nunits) {
out/user/_init.asm:  for (p = freep; !(bp > p && bp < p->s.ptr); p = p->s.ptr)
out/user/_init.asm:    if (p >= p->s.ptr && (bp > p || bp < p->s.ptr))
out/user/_init.asm:  for (p = freep; !(bp > p && bp < p->s.ptr); p = p->s.ptr)
out/user/_init.asm:  if (bp + bp->s.size == p->s.ptr) {
out/user/_init.asm:    bp->s.size += p->s.ptr->s.size;
out/user/_init.asm:    bp->s.ptr = p->s.ptr->s.ptr;
out/user/_init.asm:    bp->s.ptr = p->s.ptr;
out/user/_init.asm:  if (p + p->s.size == bp) {
out/user/_init.asm:    p->s.size += bp->s.size;
out/user/_init.asm:    p->s.ptr = bp->s.ptr;
out/user/_init.asm:    p->s.ptr = bp;
out/user/_init.asm:  hp->s.size = nu;
out/user/_init.asm:  for (p = prevp->s.ptr;; prevp = p, p = p->s.ptr) {
out/user/_init.asm:    if (p->s.size >= nunits) {
out/user/_init.asm:      if (p->s.size == nunits)
out/user/_init.asm:        prevp->s.ptr = p->s.ptr;
out/user/_init.asm:        p->s.size -= nunits;
out/user/_init.asm:        p += p->s.size;
out/user/_init.asm:        p->s.size = nunits;
out/user/_init.asm:  for (p = prevp->s.ptr;; prevp = p, p = p->s.ptr) {
out/user/_init.asm:    if (p->s.size >= nunits) {
out/user/_lab3init.asm:  for (p = freep; !(bp > p && bp < p->s.ptr); p = p->s.ptr)
out/user/_lab3init.asm:    if (p >= p->s.ptr && (bp > p || bp < p->s.ptr))
out/user/_lab3init.asm:  for (p = freep; !(bp > p && bp < p->s.ptr); p = p->s.ptr)
out/user/_lab3init.asm:  if (bp + bp->s.size == p->s.ptr) {
out/user/_lab3init.asm:    bp->s.size += p->s.ptr->s.size;
out/user/_lab3init.asm:    bp->s.ptr = p->s.ptr->s.ptr;
out/user/_lab3init.asm:    bp->s.ptr = p->s.ptr;
out/user/_lab3init.asm:  if (p + p->s.size == bp) {
out/user/_lab3init.asm:    p->s.size += bp->s.size;
out/user/_lab3init.asm:    p->s.ptr = bp->s.ptr;
out/user/_lab3init.asm:    p->s.ptr = bp;
out/user/_lab3init.asm:  hp->s.size = nu;
out/user/_lab3init.asm:  for (p = prevp->s.ptr;; prevp = p, p = p->s.ptr) {
out/user/_lab3init.asm:    if (p->s.size >= nunits) {
out/user/_lab3init.asm:      if (p->s.size == nunits)
out/user/_lab3init.asm:        prevp->s.ptr = p->s.ptr;
out/user/_lab3init.asm:        p->s.size -= nunits;
out/user/_lab3init.asm:        p += p->s.size;
out/user/_lab3init.asm:        p->s.size = nunits;
out/user/_lab3init.asm:  for (p = prevp->s.ptr;; prevp = p, p = p->s.ptr) {
out/user/_lab3init.asm:    if (p->s.size >= nunits) {
out/user/_zombie.asm:  for (p = freep; !(bp > p && bp < p->s.ptr); p = p->s.ptr)
out/user/_zombie.asm:    if (p >= p->s.ptr && (bp > p || bp < p->s.ptr))
out/user/_zombie.asm:  for (p = freep; !(bp > p && bp < p->s.ptr); p = p->s.ptr)
out/user/_zombie.asm:  if (bp + bp->s.size == p->s.ptr) {
out/user/_zombie.asm:    bp->s.size += p->s.ptr->s.size;
out/user/_zombie.asm:    bp->s.ptr = p->s.ptr->s.ptr;
out/user/_zombie.asm:    bp->s.ptr = p->s.ptr;
out/user/_zombie.asm:  if (p + p->s.size == bp) {
out/user/_zombie.asm:    p->s.size += bp->s.size;
out/user/_zombie.asm:    p->s.ptr = bp->s.ptr;
out/user/_zombie.asm:    p->s.ptr = bp;
out/user/_zombie.asm:  hp->s.size = nu;
out/user/_zombie.asm:  for (p = prevp->s.ptr;; prevp = p, p = p->s.ptr) {
out/user/_zombie.asm:    if (p->s.size >= nunits) {
out/user/_zombie.asm:      if (p->s.size == nunits)
out/user/_zombie.asm:        prevp->s.ptr = p->s.ptr;
out/user/_zombie.asm:        p->s.size -= nunits;
out/user/_zombie.asm:        p += p->s.size;
out/user/_zombie.asm:        p->s.size = nunits;
out/user/_zombie.asm:  for (p = prevp->s.ptr;; prevp = p, p = p->s.ptr) {
out/user/_zombie.asm:    if (p->s.size >= nunits) {
out/user/_sh.asm:  for (p = freep; !(bp > p && bp < p->s.ptr); p = p->s.ptr)
out/user/_sh.asm:    if (p >= p->s.ptr && (bp > p || bp < p->s.ptr))
out/user/_sh.asm:  for (p = freep; !(bp > p && bp < p->s.ptr); p = p->s.ptr)
out/user/_sh.asm:  if (bp + bp->s.size == p->s.ptr) {
out/user/_sh.asm:    bp->s.size += p->s.ptr->s.size;
out/user/_sh.asm:    bp->s.ptr = p->s.ptr->s.ptr;
out/user/_sh.asm:    bp->s.ptr = p->s.ptr;
out/user/_sh.asm:  if (p + p->s.size == bp) {
out/user/_sh.asm:    p->s.size += bp->s.size;
out/user/_sh.asm:    p->s.ptr = bp->s.ptr;
out/user/_sh.asm:    p->s.ptr = bp;
out/user/_sh.asm:  hp->s.size = nu;
out/user/_sh.asm:  for (p = prevp->s.ptr;; prevp = p, p = p->s.ptr) {
out/user/_sh.asm:    if (p->s.size >= nunits) {
out/user/_sh.asm:      if (p->s.size == nunits)
out/user/_sh.asm:        prevp->s.ptr = p->s.ptr;
out/user/_sh.asm:        p->s.size -= nunits;
out/user/_sh.asm:        p += p->s.size;
out/user/_sh.asm:        p->s.size = nunits;
out/user/_sh.asm:  for (p = prevp->s.ptr;; prevp = p, p = p->s.ptr) {
out/user/_sh.asm:    if (p->s.size >= nunits) {
out/user/_sysinfo.asm:  for (p = freep; !(bp > p && bp < p->s.ptr); p = p->s.ptr)
out/user/_sysinfo.asm:    if (p >= p->s.ptr && (bp > p || bp < p->s.ptr))
out/user/_sysinfo.asm:  for (p = freep; !(bp > p && bp < p->s.ptr); p = p->s.ptr)
out/user/_sysinfo.asm:  if (bp + bp->s.size == p->s.ptr) {
out/user/_sysinfo.asm:    bp->s.size += p->s.ptr->s.size;
out/user/_sysinfo.asm:    bp->s.ptr = p->s.ptr->s.ptr;
out/user/_sysinfo.asm:    bp->s.ptr = p->s.ptr;
out/user/_sysinfo.asm:  if (p + p->s.size == bp) {
out/user/_sysinfo.asm:    p->s.size += bp->s.size;
out/user/_sysinfo.asm:    p->s.ptr = bp->s.ptr;
out/user/_sysinfo.asm:    p->s.ptr = bp;
out/user/_sysinfo.asm:  hp->s.size = nu;
out/user/_sysinfo.asm:  for (p = prevp->s.ptr;; prevp = p, p = p->s.ptr) {
out/user/_sysinfo.asm:    if (p->s.size >= nunits) {
out/user/_sysinfo.asm:      if (p->s.size == nunits)
out/user/_sysinfo.asm:        prevp->s.ptr = p->s.ptr;
out/user/_sysinfo.asm:        p->s.size -= nunits;
out/user/_sysinfo.asm:        p += p->s.size;
out/user/_sysinfo.asm:        p->s.size = nunits;
out/user/_sysinfo.asm:  for (p = prevp->s.ptr;; prevp = p, p = p->s.ptr) {
out/user/_sysinfo.asm:    if (p->s.size >= nunits) {
out/user/_lab1test.asm:  for (p = freep; !(bp > p && bp < p->s.ptr); p = p->s.ptr)
out/user/_lab1test.asm:    if (p >= p->s.ptr && (bp > p || bp < p->s.ptr))
out/user/_lab1test.asm:  for (p = freep; !(bp > p && bp < p->s.ptr); p = p->s.ptr)
out/user/_lab1test.asm:  if (bp + bp->s.size == p->s.ptr) {
out/user/_lab1test.asm:    bp->s.size += p->s.ptr->s.size;
out/user/_lab1test.asm:    bp->s.ptr = p->s.ptr->s.ptr;
out/user/_lab1test.asm:    bp->s.ptr = p->s.ptr;
out/user/_lab1test.asm:  if (p + p->s.size == bp) {
out/user/_lab1test.asm:    p->s.size += bp->s.size;
out/user/_lab1test.asm:    p->s.ptr = bp->s.ptr;
out/user/_lab1test.asm:    p->s.ptr = bp;
out/user/_lab1test.asm:  hp->s.size = nu;
out/user/_lab1test.asm:  for (p = prevp->s.ptr;; prevp = p, p = p->s.ptr) {
out/user/_lab1test.asm:    if (p->s.size >= nunits) {
out/user/_lab1test.asm:      if (p->s.size == nunits)
out/user/_lab1test.asm:        prevp->s.ptr = p->s.ptr;
out/user/_lab1test.asm:        p->s.size -= nunits;
out/user/_lab1test.asm:        p += p->s.size;
out/user/_lab1test.asm:        p->s.size = nunits;
out/user/_lab1test.asm:  for (p = prevp->s.ptr;; prevp = p, p = p->s.ptr) {
out/user/_lab1test.asm:    if (p->s.size >= nunits) {
out/user/_lab4test_b.asm:  for (p = freep; !(bp > p && bp < p->s.ptr); p = p->s.ptr)
out/user/_lab4test_b.asm:    if (p >= p->s.ptr && (bp > p || bp < p->s.ptr))
out/user/_lab4test_b.asm:  for (p = freep; !(bp > p && bp < p->s.ptr); p = p->s.ptr)
out/user/_lab4test_b.asm:  if (bp + bp->s.size == p->s.ptr) {
out/user/_lab4test_b.asm:    bp->s.size += p->s.ptr->s.size;
out/user/_lab4test_b.asm:    bp->s.ptr = p->s.ptr->s.ptr;
out/user/_lab4test_b.asm:    bp->s.ptr = p->s.ptr;
out/user/_lab4test_b.asm:  if (p + p->s.size == bp) {
out/user/_lab4test_b.asm:    p->s.size += bp->s.size;
out/user/_lab4test_b.asm:    p->s.ptr = bp->s.ptr;
out/user/_lab4test_b.asm:    p->s.ptr = bp;
out/user/_lab4test_b.asm:  hp->s.size = nu;
out/user/_lab4test_b.asm:  for (p = prevp->s.ptr;; prevp = p, p = p->s.ptr) {
out/user/_lab4test_b.asm:    if (p->s.size >= nunits) {
out/user/_lab4test_b.asm:      if (p->s.size == nunits)
out/user/_lab4test_b.asm:        prevp->s.ptr = p->s.ptr;
out/user/_lab4test_b.asm:        p->s.size -= nunits;
out/user/_lab4test_b.asm:        p += p->s.size;
out/user/_lab4test_b.asm:        p->s.size = nunits;
out/user/_lab4test_b.asm:  for (p = prevp->s.ptr;; prevp = p, p = p->s.ptr) {
out/user/_lab4test_b.asm:    if (p->s.size >= nunits) {
out/user/_stressfs.asm:  for (p = freep; !(bp > p && bp < p->s.ptr); p = p->s.ptr)
out/user/_stressfs.asm:    if (p >= p->s.ptr && (bp > p || bp < p->s.ptr))
out/user/_stressfs.asm:  for (p = freep; !(bp > p && bp < p->s.ptr); p = p->s.ptr)
out/user/_stressfs.asm:  if (bp + bp->s.size == p->s.ptr) {
out/user/_stressfs.asm:    bp->s.size += p->s.ptr->s.size;
out/user/_stressfs.asm:    bp->s.ptr = p->s.ptr->s.ptr;
out/user/_stressfs.asm:    bp->s.ptr = p->s.ptr;
out/user/_stressfs.asm:  if (p + p->s.size == bp) {
out/user/_stressfs.asm:    p->s.size += bp->s.size;
out/user/_stressfs.asm:    p->s.ptr = bp->s.ptr;
out/user/_stressfs.asm:    p->s.ptr = bp;
out/user/_stressfs.asm:  hp->s.size = nu;
out/user/_stressfs.asm:  for (p = prevp->s.ptr;; prevp = p, p = p->s.ptr) {
out/user/_stressfs.asm:    if (p->s.size >= nunits) {
out/user/_stressfs.asm:      if (p->s.size == nunits)
out/user/_stressfs.asm:        prevp->s.ptr = p->s.ptr;
out/user/_stressfs.asm:        p->s.size -= nunits;
out/user/_stressfs.asm:        p += p->s.size;
out/user/_stressfs.asm:        p->s.size = nunits;
out/user/_stressfs.asm:  for (p = prevp->s.ptr;; prevp = p, p = p->s.ptr) {
out/user/_stressfs.asm:    if (p->s.size >= nunits) {
out/user/_kill.asm:  for (p = freep; !(bp > p && bp < p->s.ptr); p = p->s.ptr)
out/user/_kill.asm:    if (p >= p->s.ptr && (bp > p || bp < p->s.ptr))
out/user/_kill.asm:  for (p = freep; !(bp > p && bp < p->s.ptr); p = p->s.ptr)
out/user/_kill.asm:  if (bp + bp->s.size == p->s.ptr) {
out/user/_kill.asm:    bp->s.size += p->s.ptr->s.size;
out/user/_kill.asm:    bp->s.ptr = p->s.ptr->s.ptr;
out/user/_kill.asm:    bp->s.ptr = p->s.ptr;
out/user/_kill.asm:  if (p + p->s.size == bp) {
out/user/_kill.asm:    p->s.size += bp->s.size;
out/user/_kill.asm:    p->s.ptr = bp->s.ptr;
out/user/_kill.asm:    p->s.ptr = bp;
out/user/_kill.asm:  hp->s.size = nu;
out/user/_kill.asm:  for (p = prevp->s.ptr;; prevp = p, p = p->s.ptr) {
out/user/_kill.asm:    if (p->s.size >= nunits) {
out/user/_kill.asm:      if (p->s.size == nunits)
out/user/_kill.asm:        prevp->s.ptr = p->s.ptr;
out/user/_kill.asm:        p->s.size -= nunits;
out/user/_kill.asm:        p += p->s.size;
out/user/_kill.asm:        p->s.size = nunits;
out/user/_kill.asm:  for (p = prevp->s.ptr;; prevp = p, p = p->s.ptr) {
out/user/_kill.asm:    if (p->s.size >= nunits) {
out/user/_lab4test_c.asm:  for (p = freep; !(bp > p && bp < p->s.ptr); p = p->s.ptr)
out/user/_lab4test_c.asm:    if (p >= p->s.ptr && (bp > p || bp < p->s.ptr))
out/user/_lab4test_c.asm:  for (p = freep; !(bp > p && bp < p->s.ptr); p = p->s.ptr)
out/user/_lab4test_c.asm:  if (bp + bp->s.size == p->s.ptr) {
out/user/_lab4test_c.asm:    bp->s.size += p->s.ptr->s.size;
out/user/_lab4test_c.asm:    bp->s.ptr = p->s.ptr->s.ptr;
out/user/_lab4test_c.asm:    bp->s.ptr = p->s.ptr;
out/user/_lab4test_c.asm:  if (p + p->s.size == bp) {
out/user/_lab4test_c.asm:    p->s.size += bp->s.size;
out/user/_lab4test_c.asm:    p->s.ptr = bp->s.ptr;
out/user/_lab4test_c.asm:    p->s.ptr = bp;
out/user/_lab4test_c.asm:  hp->s.size = nu;
out/user/_lab4test_c.asm:  for (p = prevp->s.ptr;; prevp = p, p = p->s.ptr) {
out/user/_lab4test_c.asm:    if (p->s.size >= nunits) {
out/user/_lab4test_c.asm:      if (p->s.size == nunits)
out/user/_lab4test_c.asm:        prevp->s.ptr = p->s.ptr;
out/user/_lab4test_c.asm:        p->s.size -= nunits;
out/user/_lab4test_c.asm:        p += p->s.size;
out/user/_lab4test_c.asm:        p->s.size = nunits;
out/user/_lab4test_c.asm:  for (p = prevp->s.ptr;; prevp = p, p = p->s.ptr) {
out/user/_lab4test_c.asm:    if (p->s.size >= nunits) {
out/user/_ln.asm:  for (p = freep; !(bp > p && bp < p->s.ptr); p = p->s.ptr)
out/user/_ln.asm:    if (p >= p->s.ptr && (bp > p || bp < p->s.ptr))
out/user/_ln.asm:  for (p = freep; !(bp > p && bp < p->s.ptr); p = p->s.ptr)
out/user/_ln.asm:  if (bp + bp->s.size == p->s.ptr) {
out/user/_ln.asm:    bp->s.size += p->s.ptr->s.size;
out/user/_ln.asm:    bp->s.ptr = p->s.ptr->s.ptr;
out/user/_ln.asm:    bp->s.ptr = p->s.ptr;
out/user/_ln.asm:  if (p + p->s.size == bp) {
out/user/_ln.asm:    p->s.size += bp->s.size;
out/user/_ln.asm:    p->s.ptr = bp->s.ptr;
out/user/_ln.asm:    p->s.ptr = bp;
out/user/_ln.asm:  hp->s.size = nu;
out/user/_ln.asm:  for (p = prevp->s.ptr;; prevp = p, p = p->s.ptr) {
out/user/_ln.asm:    if (p->s.size >= nunits) {
out/user/_ln.asm:      if (p->s.size == nunits)
out/user/_ln.asm:        prevp->s.ptr = p->s.ptr;
out/user/_ln.asm:        p->s.size -= nunits;
out/user/_ln.asm:        p += p->s.size;
out/user/_ln.asm:        p->s.size = nunits;
out/user/_ln.asm:  for (p = prevp->s.ptr;; prevp = p, p = p->s.ptr) {
out/user/_ln.asm:    if (p->s.size >= nunits) {
out/user/_echo.asm:  for (p = freep; !(bp > p && bp < p->s.ptr); p = p->s.ptr)
out/user/_echo.asm:    if (p >= p->s.ptr && (bp > p || bp < p->s.ptr))
out/user/_echo.asm:  for (p = freep; !(bp > p && bp < p->s.ptr); p = p->s.ptr)
out/user/_echo.asm:  if (bp + bp->s.size == p->s.ptr) {
out/user/_echo.asm:    bp->s.size += p->s.ptr->s.size;
out/user/_echo.asm:    bp->s.ptr = p->s.ptr->s.ptr;
out/user/_echo.asm:    bp->s.ptr = p->s.ptr;
out/user/_echo.asm:  if (p + p->s.size == bp) {
out/user/_echo.asm:    p->s.size += bp->s.size;
out/user/_echo.asm:    p->s.ptr = bp->s.ptr;
out/user/_echo.asm:    p->s.ptr = bp;
out/user/_echo.asm:  hp->s.size = nu;
out/user/_echo.asm:  for (p = prevp->s.ptr;; prevp = p, p = p->s.ptr) {
out/user/_echo.asm:    if (p->s.size >= nunits) {
out/user/_echo.asm:      if (p->s.size == nunits)
out/user/_echo.asm:        prevp->s.ptr = p->s.ptr;
out/user/_echo.asm:        p->s.size -= nunits;
out/user/_echo.asm:        p += p->s.size;
out/user/_echo.asm:        p->s.size = nunits;
out/user/_echo.asm:  for (p = prevp->s.ptr;; prevp = p, p = p->s.ptr) {
out/user/_echo.asm:    if (p->s.size >= nunits) {
out/user/_cat.asm:  for (p = freep; !(bp > p && bp < p->s.ptr); p = p->s.ptr)
out/user/_cat.asm:    if (p >= p->s.ptr && (bp > p || bp < p->s.ptr))
out/user/_cat.asm:  for (p = freep; !(bp > p && bp < p->s.ptr); p = p->s.ptr)
out/user/_cat.asm:  if (bp + bp->s.size == p->s.ptr) {
out/user/_cat.asm:    bp->s.size += p->s.ptr->s.size;
out/user/_cat.asm:    bp->s.ptr = p->s.ptr->s.ptr;
out/user/_cat.asm:    bp->s.ptr = p->s.ptr;
out/user/_cat.asm:  if (p + p->s.size == bp) {
out/user/_cat.asm:    p->s.size += bp->s.size;
out/user/_cat.asm:    p->s.ptr = bp->s.ptr;
out/user/_cat.asm:    p->s.ptr = bp;
out/user/_cat.asm:  hp->s.size = nu;
out/user/_cat.asm:  for (p = prevp->s.ptr;; prevp = p, p = p->s.ptr) {
out/user/_cat.asm:    if (p->s.size >= nunits) {
out/user/_cat.asm:      if (p->s.size == nunits)
out/user/_cat.asm:        prevp->s.ptr = p->s.ptr;
out/user/_cat.asm:        p->s.size -= nunits;
out/user/_cat.asm:        p += p->s.size;
out/user/_cat.asm:        p->s.size = nunits;
out/user/_cat.asm:  for (p = prevp->s.ptr;; prevp = p, p = p->s.ptr) {
out/user/_cat.asm:    if (p->s.size >= nunits) {
out/user/_lab2test.asm:  for (p = freep; !(bp > p && bp < p->s.ptr); p = p->s.ptr)
out/user/_lab2test.asm:    if (p >= p->s.ptr && (bp > p || bp < p->s.ptr))
out/user/_lab2test.asm:  for (p = freep; !(bp > p && bp < p->s.ptr); p = p->s.ptr)
out/user/_lab2test.asm:  if (bp + bp->s.size == p->s.ptr) {
out/user/_lab2test.asm:    bp->s.size += p->s.ptr->s.size;
out/user/_lab2test.asm:    bp->s.ptr = p->s.ptr->s.ptr;
out/user/_lab2test.asm:    bp->s.ptr = p->s.ptr;
out/user/_lab2test.asm:  if (p + p->s.size == bp) {
out/user/_lab2test.asm:    p->s.size += bp->s.size;
out/user/_lab2test.asm:    p->s.ptr = bp->s.ptr;
out/user/_lab2test.asm:    p->s.ptr = bp;
out/user/_lab2test.asm:  hp->s.size = nu;
out/user/_lab2test.asm:  for (p = prevp->s.ptr;; prevp = p, p = p->s.ptr) {
out/user/_lab2test.asm:    if (p->s.size >= nunits) {
out/user/_lab2test.asm:      if (p->s.size == nunits)
out/user/_lab2test.asm:        prevp->s.ptr = p->s.ptr;
out/user/_lab2test.asm:        p->s.size -= nunits;
out/user/_lab2test.asm:        p += p->s.size;
out/user/_lab2test.asm:        p->s.size = nunits;
out/user/_lab2test.asm:  for (p = prevp->s.ptr;; prevp = p, p = p->s.ptr) {
out/user/_lab2test.asm:    if (p->s.size >= nunits) {
out/user/_wc.asm:  for (p = freep; !(bp > p && bp < p->s.ptr); p = p->s.ptr)
out/user/_wc.asm:    if (p >= p->s.ptr && (bp > p || bp < p->s.ptr))
out/user/_wc.asm:  for (p = freep; !(bp > p && bp < p->s.ptr); p = p->s.ptr)
out/user/_wc.asm:  if (bp + bp->s.size == p->s.ptr) {
out/user/_wc.asm:    bp->s.size += p->s.ptr->s.size;
out/user/_wc.asm:    bp->s.ptr = p->s.ptr->s.ptr;
out/user/_wc.asm:    bp->s.ptr = p->s.ptr;
out/user/_wc.asm:  if (p + p->s.size == bp) {
out/user/_wc.asm:    p->s.size += bp->s.size;
out/user/_wc.asm:    p->s.ptr = bp->s.ptr;
out/user/_wc.asm:    p->s.ptr = bp;
out/user/_wc.asm:  hp->s.size = nu;
out/user/_wc.asm:  for (p = prevp->s.ptr;; prevp = p, p = p->s.ptr) {
out/user/_wc.asm:    if (p->s.size >= nunits) {
out/user/_wc.asm:      if (p->s.size == nunits)
out/user/_wc.asm:        prevp->s.ptr = p->s.ptr;
out/user/_wc.asm:        p->s.size -= nunits;
out/user/_wc.asm:        p += p->s.size;
out/user/_wc.asm:        p->s.size = nunits;
out/user/_wc.asm:  for (p = prevp->s.ptr;; prevp = p, p = p->s.ptr) {
out/user/_wc.asm:    if (p->s.size >= nunits) {
out/user/_grep.asm:  for (p = freep; !(bp > p && bp < p->s.ptr); p = p->s.ptr)
out/user/_grep.asm:    if (p >= p->s.ptr && (bp > p || bp < p->s.ptr))
out/user/_grep.asm:  for (p = freep; !(bp > p && bp < p->s.ptr); p = p->s.ptr)
out/user/_grep.asm:  if (bp + bp->s.size == p->s.ptr) {
out/user/_grep.asm:    bp->s.size += p->s.ptr->s.size;
out/user/_grep.asm:    bp->s.ptr = p->s.ptr->s.ptr;
out/user/_grep.asm:    bp->s.ptr = p->s.ptr;
out/user/_grep.asm:  if (p + p->s.size == bp) {
out/user/_grep.asm:    p->s.size += bp->s.size;
out/user/_grep.asm:    p->s.ptr = bp->s.ptr;
out/user/_grep.asm:    p->s.ptr = bp;
out/user/_grep.asm:  hp->s.size = nu;
out/user/_grep.asm:  for (p = prevp->s.ptr;; prevp = p, p = p->s.ptr) {
out/user/_grep.asm:    if (p->s.size >= nunits) {
out/user/_grep.asm:      if (p->s.size == nunits)
out/user/_grep.asm:        prevp->s.ptr = p->s.ptr;
out/user/_grep.asm:        p->s.size -= nunits;
out/user/_grep.asm:        p += p->s.size;
out/user/_grep.asm:        p->s.size = nunits;
out/user/_grep.asm:  for (p = prevp->s.ptr;; prevp = p, p = p->s.ptr) {
out/user/_grep.asm:    if (p->s.size >= nunits) {
out/user/_ls.asm:  for (p = path + strlen(path); p >= path && *p != '/'; p--)
out/user/_ls.asm:  for (p = freep; !(bp > p && bp < p->s.ptr); p = p->s.ptr)
out/user/_ls.asm:    if (p >= p->s.ptr && (bp > p || bp < p->s.ptr))
out/user/_ls.asm:  for (p = freep; !(bp > p && bp < p->s.ptr); p = p->s.ptr)
out/user/_ls.asm:  if (bp + bp->s.size == p->s.ptr) {
out/user/_ls.asm:    bp->s.size += p->s.ptr->s.size;
out/user/_ls.asm:    bp->s.ptr = p->s.ptr->s.ptr;
out/user/_ls.asm:    bp->s.ptr = p->s.ptr;
out/user/_ls.asm:  if (p + p->s.size == bp) {
out/user/_ls.asm:    p->s.size += bp->s.size;
out/user/_ls.asm:    p->s.ptr = bp->s.ptr;
out/user/_ls.asm:    p->s.ptr = bp;
out/user/_ls.asm:  hp->s.size = nu;
out/user/_ls.asm:  for (p = prevp->s.ptr;; prevp = p, p = p->s.ptr) {
out/user/_ls.asm:    if (p->s.size >= nunits) {
out/user/_ls.asm:      if (p->s.size == nunits)
out/user/_ls.asm:        prevp->s.ptr = p->s.ptr;
out/user/_ls.asm:        p->s.size -= nunits;
out/user/_ls.asm:        p += p->s.size;
out/user/_ls.asm:        p->s.size = nunits;
out/user/_ls.asm:  for (p = prevp->s.ptr;; prevp = p, p = p->s.ptr) {
out/user/_ls.asm:    if (p->s.size >= nunits) {
out/user/_rm.asm:  for (p = freep; !(bp > p && bp < p->s.ptr); p = p->s.ptr)
out/user/_rm.asm:    if (p >= p->s.ptr && (bp > p || bp < p->s.ptr))
out/user/_rm.asm:  for (p = freep; !(bp > p && bp < p->s.ptr); p = p->s.ptr)
out/user/_rm.asm:  if (bp + bp->s.size == p->s.ptr) {
out/user/_rm.asm:    bp->s.size += p->s.ptr->s.size;
out/user/_rm.asm:    bp->s.ptr = p->s.ptr->s.ptr;
out/user/_rm.asm:    bp->s.ptr = p->s.ptr;
out/user/_rm.asm:  if (p + p->s.size == bp) {
out/user/_rm.asm:    p->s.size += bp->s.size;
out/user/_rm.asm:    p->s.ptr = bp->s.ptr;
out/user/_rm.asm:    p->s.ptr = bp;
out/user/_rm.asm:  hp->s.size = nu;
out/user/_rm.asm:  for (p = prevp->s.ptr;; prevp = p, p = p->s.ptr) {
out/user/_rm.asm:    if (p->s.size >= nunits) {
out/user/_rm.asm:      if (p->s.size == nunits)
out/user/_rm.asm:        prevp->s.ptr = p->s.ptr;
out/user/_rm.asm:        p->s.size -= nunits;
out/user/_rm.asm:        p += p->s.size;
out/user/_rm.asm:        p->s.size = nunits;
out/user/_rm.asm:  for (p = prevp->s.ptr;; prevp = p, p = p->s.ptr) {
out/user/_rm.asm:    if (p->s.size >= nunits) {
out/user/_lab4test_a.asm:  for (p = freep; !(bp > p && bp < p->s.ptr); p = p->s.ptr)
out/user/_lab4test_a.asm:    if (p >= p->s.ptr && (bp > p || bp < p->s.ptr))
out/user/_lab4test_a.asm:  for (p = freep; !(bp > p && bp < p->s.ptr); p = p->s.ptr)
out/user/_lab4test_a.asm:  if (bp + bp->s.size == p->s.ptr) {
out/user/_lab4test_a.asm:    bp->s.size += p->s.ptr->s.size;
out/user/_lab4test_a.asm:    bp->s.ptr = p->s.ptr->s.ptr;
out/user/_lab4test_a.asm:    bp->s.ptr = p->s.ptr;
out/user/_lab4test_a.asm:  if (p + p->s.size == bp) {
out/user/_lab4test_a.asm:    p->s.size += bp->s.size;
out/user/_lab4test_a.asm:    p->s.ptr = bp->s.ptr;
out/user/_lab4test_a.asm:    p->s.ptr = bp;
out/user/_lab4test_a.asm:  hp->s.size = nu;
out/user/_lab4test_a.asm:  for (p = prevp->s.ptr;; prevp = p, p = p->s.ptr) {
out/user/_lab4test_a.asm:    if (p->s.size >= nunits) {
out/user/_lab4test_a.asm:      if (p->s.size == nunits)
out/user/_lab4test_a.asm:        prevp->s.ptr = p->s.ptr;
out/user/_lab4test_a.asm:        p->s.size -= nunits;
out/user/_lab4test_a.asm:        p += p->s.size;
out/user/_lab4test_a.asm:        p->s.size = nunits;
out/user/_lab4test_a.asm:  for (p = prevp->s.ptr;; prevp = p, p = p->s.ptr) {
out/user/_lab4test_a.asm:    if (p->s.size >= nunits) {
out/user/_lab3test.asm:  for (p = freep; !(bp > p && bp < p->s.ptr); p = p->s.ptr)
out/user/_lab3test.asm:    if (p >= p->s.ptr && (bp > p || bp < p->s.ptr))
out/user/_lab3test.asm:  for (p = freep; !(bp > p && bp < p->s.ptr); p = p->s.ptr)
out/user/_lab3test.asm:  if (bp + bp->s.size == p->s.ptr) {
out/user/_lab3test.asm:    bp->s.size += p->s.ptr->s.size;
out/user/_lab3test.asm:    bp->s.ptr = p->s.ptr->s.ptr;
out/user/_lab3test.asm:    bp->s.ptr = p->s.ptr;
out/user/_lab3test.asm:  if (p + p->s.size == bp) {
out/user/_lab3test.asm:    p->s.size += bp->s.size;
out/user/_lab3test.asm:    p->s.ptr = bp->s.ptr;
out/user/_lab3test.asm:    p->s.ptr = bp;
out/user/_lab3test.asm:  hp->s.size = nu;
out/user/_lab3test.asm:  for (p = prevp->s.ptr;; prevp = p, p = p->s.ptr) {
out/user/_lab3test.asm:    if (p->s.size >= nunits) {
out/user/_lab3test.asm:      if (p->s.size == nunits)
out/user/_lab3test.asm:        prevp->s.ptr = p->s.ptr;
out/user/_lab3test.asm:        p->s.size -= nunits;
out/user/_lab3test.asm:        p += p->s.size;
out/user/_lab3test.asm:        p->s.size = nunits;
out/user/_lab3test.asm:  for (p = prevp->s.ptr;; prevp = p, p = p->s.ptr) {
out/user/_lab3test.asm:    if (p->s.size >= nunits) {
out/xk.asm:    if(p->pftable[pfd]==NULL) { //TODO Not sure how to check is emtpty
out/xk.asm:      p->pftable[pfd] = &ftable[gfd];
out/xk.asm:  if(p->pftable[fd] == NULL)
out/xk.asm:  struct file_info f = *(p->pftable[fd]);
out/xk.asm:   if(p->pftable[fd]==NULL)return -1;
out/xk.asm:   struct file_info f=*(p->pftable[fd]);
out/xk.asm:     p->pftable[fd]->ref -= 1;
out/xk.asm:     p->pftable[fd]->iptr = 0;
out/xk.asm:     p->pftable[fd]->ref = 0;
out/xk.asm:     p->pftable[fd]->path = 0;
out/xk.asm:     p->pftable[fd]->access_permission = 0;
out/xk.asm:     p->pftable[fd]->offset=0;
out/xk.asm:   //   cprintf("%s close  for fd =%d and ref is %d \n",f.path,fd,p->pftable[fd]->ref);
out/xk.asm:   p->pftable[fd] = NULL;
out/xk.asm:   if(p->pftable[fd]==NULL)return -1;
out/xk.asm:   struct file_info f=*(p->pftable[fd]);
out/xk.asm:   p->pftable[fd]->offset+=offset;
out/xk.asm:   //cprintf("offset right now %d and try to read %d bytes and got %d read \n",p->pftable[fd]->offset,bytes_read,offset);
out/xk.asm:   if(p->pftable[fd]==NULL)return -1;
out/xk.asm:   struct file_info f=*(p->pftable[fd]);
out/xk.asm:   if(p->pftable[fd]==NULL)return -1;
out/xk.asm:   struct file_info f=*(p->pftable[fd]);
out/xk.asm:    if(p->pftable[i] == NULL) {
out/xk.asm:      p->pftable[i] = p->pftable[fd]; 
out/xk.asm:      p->pftable[fd]->ref++;     //increase reference count
out/xk.asm:      //cprintf("dup file:%s from fd = %d to new fd = %d and ref become %d \n",f.path,fd,i,p->pftable[fd]->ref);
out/xk.asm:  memmove(sb, bp->data, sizeof(*sb));
out/xk.asm:    if (ip->ref > 0 && ip->dev == dev && ip->inum == inum) {
out/xk.asm:      ip->ref++;
out/xk.asm:    if (empty == 0 && ip->ref == 0) // Remember empty slot.
out/xk.asm:  ip->ref = 1;
out/xk.asm:  ip->valid = 0;
out/xk.asm:  ip->dev = dev;
out/xk.asm:  ip->inum = inum;
out/xk.asm:  ip->ref++;
out/xk.asm:  if (ip->ref == 1)
out/xk.asm:    ip->type = 0;
out/xk.asm:  ip->ref--;
out/xk.asm:  if(ip == 0 || ip->ref < 1)
out/xk.asm:  acquiresleep(&ip->lock);
out/xk.asm:  if (ip->valid == 0) {
out/xk.asm:    read_dinode(ip->inum, &dip);
out/xk.asm:    ip->type = dip.type;
out/xk.asm:    ip->devid = dip.devid;
out/xk.asm:    ip->size = dip.size;
out/xk.asm:    ip->data = dip.data;
out/xk.asm:    ip->valid = 1;
out/xk.asm:    if (ip->type == 0)
out/xk.asm:  if(ip == 0 || !holdingsleep(&ip->lock) || ip->ref < 1)
out/xk.asm:  releasesleep(&ip->lock);
out/xk.asm:// Caller must hold ip->lock.
out/xk.asm:  if (!holdingsleep(&ip->lock))
out/xk.asm:  st->dev = ip->dev;
out/xk.asm:  st->ino = ip->inum;
out/xk.asm:  st->type = ip->type;
out/xk.asm:  st->size = ip->size;
out/xk.asm:// Caller must hold ip->lock.
out/xk.asm:  if (!holdingsleep(&ip->lock))
out/xk.asm:  if (ip->type == T_DEV) {
out/xk.asm:    if (ip->devid < 0 || ip->devid >= NDEV || !devsw[ip->devid].read)
out/xk.asm:    return devsw[ip->devid].read(ip, dst, n);
out/xk.asm:  if (off > ip->size || off + n < off)
out/xk.asm:  if (off + n > ip->size)
out/xk.asm:    n = ip->size - off;
out/xk.asm:    bp = bread(ip->dev, ip->data.startblkno + off / BSIZE);
out/xk.asm:    memmove(dst, bp->data + off % BSIZE, m);
out/xk.asm:// Caller must hold ip->lock.
out/xk.asm:  if (!holdingsleep(&ip->lock))
out/xk.asm:  if (ip->type == T_DEV) {
out/xk.asm:    if (ip->devid < 0 || ip->devid >= NDEV || !devsw[ip->devid].write)
out/xk.asm:    return devsw[ip->devid].write(ip, src, n);
out/xk.asm:  if (dp->type != T_DIR)
out/xk.asm:  for (off = 0; off < dp->size; off += sizeof(de)) {
out/xk.asm:      return iget(dp->dev, inum);
out/xk.asm:  for (off = 0; off < dp->size; off += sizeof(de)) {
out/xk.asm:    if (ip->type != T_DIR) {
out/xk.asm:  if ((mp = mpsearch()) == 0 || mp->physaddr == 0)
out/xk.asm:  conf = (struct mpconf *)P2V((uint64_t)mp->physaddr);
out/xk.asm:  if (mp->imcrp) {
out/xk.asm:    if (p->state == UNUSED)
out/xk.asm:  p->state = EMBRYO;
out/xk.asm:  p->pid = nextpid++;
out/xk.asm:  p->killed = 0;
out/xk.asm:  if ((p->kstack = kalloc()) == 0) {
out/xk.asm:    p->state = UNUSED;
out/xk.asm:  sp = p->kstack + KSTACKSIZE;
out/xk.asm:  sp -= sizeof *p->tf;
out/xk.asm:  p->tf = (struct trap_frame *)sp;
out/xk.asm:  sp -= sizeof *p->context;
out/xk.asm:  p->context = (struct context *)sp;
out/xk.asm:  memset(p->context, 0, sizeof *p->context);
out/xk.asm:  p->context->rip = (uint64_t)forkret;
out/xk.asm:  assertm(vspaceinit(&p->vspace) == 0, "error initializing process's virtual address descriptor");
out/xk.asm:  vspaceinitcode(&p->vspace, _binary_out_initcode_start, (int64_t)_binary_out_initcode_size);
out/xk.asm:  memset(p->tf, 0, sizeof(*p->tf));
out/xk.asm:  p->tf->cs = (SEG_UCODE << 3) | DPL_USER;
out/xk.asm:  p->tf->ss = (SEG_UDATA << 3) | DPL_USER;
out/xk.asm:  p->tf->rflags = FLAGS_IF;
out/xk.asm:  p->tf->rip = VRBOT(&p->vspace.regions[VR_CODE]);  // beginning of initcode.S
out/xk.asm:  p->tf->rsp = VRTOP(&p->vspace.regions[VR_USTACK]);
out/xk.asm:  safestrcpy(p->name, "initcode", sizeof(p->name));
out/xk.asm:  // this assignment to p->state lets other cores
out/xk.asm:  p->state = RUNNABLE;
out/xk.asm:      if (p->state != RUNNABLE)
out/xk.asm:      p->state = RUNNING;
out/xk.asm:      swtch(&mycpu()->scheduler, p->context);
out/xk.asm:      // It should have changed its p->state before coming back.
out/xk.asm:  // change p->state and then call sched.
out/xk.asm:    if (p->state == SLEEPING && p->chan == chan)
out/xk.asm:      p->state = RUNNABLE;
out/xk.asm:    if (p->pid == pid) {
out/xk.asm:      p->killed = 1;
out/xk.asm:      if (p->state == SLEEPING)
out/xk.asm:        p->state = RUNNABLE;
out/xk.asm:    if (p->state == UNUSED)
out/xk.asm:    if (p->state != 0 && p->state < NELEM(states) && states[p->state])
out/xk.asm:      state = states[p->state];
out/xk.asm:    cprintf("%d %s %s", p->pid, state, p->name);
out/xk.asm:    if (p->state == SLEEPING) {
out/xk.asm:      getcallerpcs((uint64_t *)p->context->rbp, pc);
out/xk.asm:    if (p->pid == pid)
out/xk.asm:  if (!p->kstack)
out/xk.asm:  if (!p->vspace.pgtbl)
out/xk.asm:  mycpu()->ts.rsp0 = (uint64_t)p->kstack + KSTACKSIZE;
out/xk.asm:  lcr3(V2P(p->vspace.pgtbl));
out/xk_memfs.asm:    if(p->pftable[pfd]==NULL) { //TODO Not sure how to check is emtpty
out/xk_memfs.asm:      p->pftable[pfd] = &ftable[gfd];
out/xk_memfs.asm:  if(p->pftable[fd] == NULL)
out/xk_memfs.asm:  struct file_info f = *(p->pftable[fd]);
out/xk_memfs.asm:   if(p->pftable[fd]==NULL)return -1;
out/xk_memfs.asm:   struct file_info f=*(p->pftable[fd]);
out/xk_memfs.asm:     p->pftable[fd]->ref -= 1;
out/xk_memfs.asm:     p->pftable[fd]->iptr = 0;
out/xk_memfs.asm:     p->pftable[fd]->ref = 0;
out/xk_memfs.asm:     p->pftable[fd]->path = 0;
out/xk_memfs.asm:     p->pftable[fd]->access_permission = 0;
out/xk_memfs.asm:     p->pftable[fd]->offset=0;
out/xk_memfs.asm:   //   cprintf("%s close  for fd =%d and ref is %d \n",f.path,fd,p->pftable[fd]->ref);
out/xk_memfs.asm:   p->pftable[fd] = NULL;
out/xk_memfs.asm:   if(p->pftable[fd]==NULL)return -1;
out/xk_memfs.asm:   struct file_info f=*(p->pftable[fd]);
out/xk_memfs.asm:   p->pftable[fd]->offset+=offset;
out/xk_memfs.asm:   //cprintf("offset right now %d and try to read %d bytes and got %d read \n",p->pftable[fd]->offset,bytes_read,offset);
out/xk_memfs.asm:   if(p->pftable[fd]==NULL)return -1;
out/xk_memfs.asm:   struct file_info f=*(p->pftable[fd]);
out/xk_memfs.asm:   if(p->pftable[fd]==NULL)return -1;
out/xk_memfs.asm:   struct file_info f=*(p->pftable[fd]);
out/xk_memfs.asm:    if(p->pftable[i] == NULL) {
out/xk_memfs.asm:      p->pftable[i] = p->pftable[fd]; 
out/xk_memfs.asm:      p->pftable[fd]->ref++;     //increase reference count
out/xk_memfs.asm:      //cprintf("dup file:%s from fd = %d to new fd = %d and ref become %d \n",f.path,fd,i,p->pftable[fd]->ref);
out/xk_memfs.asm:  memmove(sb, bp->data, sizeof(*sb));
out/xk_memfs.asm:    if (ip->ref > 0 && ip->dev == dev && ip->inum == inum) {
out/xk_memfs.asm:      ip->ref++;
out/xk_memfs.asm:    if (empty == 0 && ip->ref == 0) // Remember empty slot.
out/xk_memfs.asm:  ip->ref = 1;
out/xk_memfs.asm:  ip->valid = 0;
out/xk_memfs.asm:  ip->dev = dev;
out/xk_memfs.asm:  ip->inum = inum;
out/xk_memfs.asm:  ip->ref++;
out/xk_memfs.asm:  if (ip->ref == 1)
out/xk_memfs.asm:    ip->type = 0;
out/xk_memfs.asm:  ip->ref--;
out/xk_memfs.asm:  if(ip == 0 || ip->ref < 1)
out/xk_memfs.asm:  acquiresleep(&ip->lock);
out/xk_memfs.asm:  if (ip->valid == 0) {
out/xk_memfs.asm:    read_dinode(ip->inum, &dip);
out/xk_memfs.asm:    ip->type = dip.type;
out/xk_memfs.asm:    ip->devid = dip.devid;
out/xk_memfs.asm:    ip->size = dip.size;
out/xk_memfs.asm:    ip->data = dip.data;
out/xk_memfs.asm:    ip->valid = 1;
out/xk_memfs.asm:    if (ip->type == 0)
out/xk_memfs.asm:  if(ip == 0 || !holdingsleep(&ip->lock) || ip->ref < 1)
out/xk_memfs.asm:  releasesleep(&ip->lock);
out/xk_memfs.asm:// Caller must hold ip->lock.
out/xk_memfs.asm:  if (!holdingsleep(&ip->lock))
out/xk_memfs.asm:  st->dev = ip->dev;
out/xk_memfs.asm:  st->ino = ip->inum;
out/xk_memfs.asm:  st->type = ip->type;
out/xk_memfs.asm:  st->size = ip->size;
out/xk_memfs.asm:// Caller must hold ip->lock.
out/xk_memfs.asm:  if (!holdingsleep(&ip->lock))
out/xk_memfs.asm:  if (ip->type == T_DEV) {
out/xk_memfs.asm:    if (ip->devid < 0 || ip->devid >= NDEV || !devsw[ip->devid].read)
out/xk_memfs.asm:    return devsw[ip->devid].read(ip, dst, n);
out/xk_memfs.asm:  if (off > ip->size || off + n < off)
out/xk_memfs.asm:  if (off + n > ip->size)
out/xk_memfs.asm:    n = ip->size - off;
out/xk_memfs.asm:    bp = bread(ip->dev, ip->data.startblkno + off / BSIZE);
out/xk_memfs.asm:    memmove(dst, bp->data + off % BSIZE, m);
out/xk_memfs.asm:// Caller must hold ip->lock.
out/xk_memfs.asm:  if (!holdingsleep(&ip->lock))
out/xk_memfs.asm:  if (ip->type == T_DEV) {
out/xk_memfs.asm:    if (ip->devid < 0 || ip->devid >= NDEV || !devsw[ip->devid].write)
out/xk_memfs.asm:    return devsw[ip->devid].write(ip, src, n);
out/xk_memfs.asm:  if (dp->type != T_DIR)
out/xk_memfs.asm:  for (off = 0; off < dp->size; off += sizeof(de)) {
out/xk_memfs.asm:      return iget(dp->dev, inum);
out/xk_memfs.asm:  for (off = 0; off < dp->size; off += sizeof(de)) {
out/xk_memfs.asm:    if (ip->type != T_DIR) {
out/xk_memfs.asm:  if ((mp = mpsearch()) == 0 || mp->physaddr == 0)
out/xk_memfs.asm:  conf = (struct mpconf *)P2V((uint64_t)mp->physaddr);
out/xk_memfs.asm:  if (mp->imcrp) {
out/xk_memfs.asm:    if (p->state == UNUSED)
out/xk_memfs.asm:  p->state = EMBRYO;
out/xk_memfs.asm:  p->pid = nextpid++;
out/xk_memfs.asm:  p->killed = 0;
out/xk_memfs.asm:  if ((p->kstack = kalloc()) == 0) {
out/xk_memfs.asm:    p->state = UNUSED;
out/xk_memfs.asm:  sp = p->kstack + KSTACKSIZE;
out/xk_memfs.asm:  sp -= sizeof *p->tf;
out/xk_memfs.asm:  p->tf = (struct trap_frame *)sp;
out/xk_memfs.asm:  sp -= sizeof *p->context;
out/xk_memfs.asm:  p->context = (struct context *)sp;
out/xk_memfs.asm:  memset(p->context, 0, sizeof *p->context);
out/xk_memfs.asm:  p->context->rip = (uint64_t)forkret;
out/xk_memfs.asm:  assertm(vspaceinit(&p->vspace) == 0, "error initializing process's virtual address descriptor");
out/xk_memfs.asm:  vspaceinitcode(&p->vspace, _binary_out_initcode_start, (int64_t)_binary_out_initcode_size);
out/xk_memfs.asm:  memset(p->tf, 0, sizeof(*p->tf));
out/xk_memfs.asm:  p->tf->cs = (SEG_UCODE << 3) | DPL_USER;
out/xk_memfs.asm:  p->tf->ss = (SEG_UDATA << 3) | DPL_USER;
out/xk_memfs.asm:  p->tf->rflags = FLAGS_IF;
out/xk_memfs.asm:  p->tf->rip = VRBOT(&p->vspace.regions[VR_CODE]);  // beginning of initcode.S
out/xk_memfs.asm:  p->tf->rsp = VRTOP(&p->vspace.regions[VR_USTACK]);
out/xk_memfs.asm:  safestrcpy(p->name, "initcode", sizeof(p->name));
out/xk_memfs.asm:  // this assignment to p->state lets other cores
out/xk_memfs.asm:  p->state = RUNNABLE;
out/xk_memfs.asm:      if (p->state != RUNNABLE)
out/xk_memfs.asm:      p->state = RUNNING;
out/xk_memfs.asm:      swtch(&mycpu()->scheduler, p->context);
out/xk_memfs.asm:      // It should have changed its p->state before coming back.
out/xk_memfs.asm:  // change p->state and then call sched.
out/xk_memfs.asm:    if (p->state == SLEEPING && p->chan == chan)
out/xk_memfs.asm:      p->state = RUNNABLE;
out/xk_memfs.asm:    if (p->pid == pid) {
out/xk_memfs.asm:      p->killed = 1;
out/xk_memfs.asm:      if (p->state == SLEEPING)
out/xk_memfs.asm:        p->state = RUNNABLE;
out/xk_memfs.asm:    if (p->state == UNUSED)
out/xk_memfs.asm:    if (p->state != 0 && p->state < NELEM(states) && states[p->state])
out/xk_memfs.asm:      state = states[p->state];
out/xk_memfs.asm:    cprintf("%d %s %s", p->pid, state, p->name);
out/xk_memfs.asm:    if (p->state == SLEEPING) {
out/xk_memfs.asm:      getcallerpcs((uint64_t *)p->context->rbp, pc);
out/xk_memfs.asm:    if (p->pid == pid)
out/xk_memfs.asm:  if (!p->kstack)
out/xk_memfs.asm:  if (!p->vspace.pgtbl)
out/xk_memfs.asm:  mycpu()->ts.rsp0 = (uint64_t)p->kstack + KSTACKSIZE;
out/xk_memfs.asm:  lcr3(V2P(p->vspace.pgtbl));
Binary file .git/objects/11/c0df7f1290bd10e4997bb4b7e6b5716fad0393 matches
Binary file .git/objects/0a/802ef47db10fec086b8d9b6123321360e4714e matches
.git/objects/44/64f9a611309c0bbec3ccdb218feb34d6c5045b:®^Þæåå¾ïu‚õø4fC”„I|r9Š×‹ÖZIb‡À¸¡Q_tp›°yÑœmôÆãŠé"™´˜¸¦‚ñdœÓÄ¤èœ¥ð^èÖàmTêp-w÷ºÓèmÉc¹Ñ(µ}Ì?ãí|³´>„ˆð¬Qk•»“ÿRR[…7ø>yÌÚÔŸ”–ù3Õ/yÅWX
Binary file .git/objects/ec/ea5e5ac69afc0c8f9790bf1a1ba874a40f60b1 matches
Binary file .git/objects/89/9a20ccdaac57d4ce1b540db30e8d5df50a0a31 matches
Binary file .git/objects/7d/93f135a194257e7e0d7e5d0e3df0d8a31f7ae8 matches
Binary file .git/objects/35/9beea93ba2a0836b9801a3f68a9805ac3bc6f0 matches
Binary file .git/objects/94/47e191ec4d612101a6305498407e26c27810d7 matches
Binary file .git/objects/93/f8fcb33c723c8f791f3f20a66e27157e8f3d99 matches
Binary file .git/objects/pack/pack-d3e623e2ca6213a0430abe580df7040b9e7001fc.pack matches
.git/rebase-apply/patch:+  struct file_info f = ftable[*(p->pftable[fd])];
.git/rebase-apply/patch:+  struct file_info f=ftable[*(p->pftable[fd])];
.git/rebase-apply/0001:+  struct file_info f = ftable[*(p->pftable[fd])];
.git/rebase-apply/0001:+  struct file_info f=ftable[*(p->pftable[fd])];
Binary file lab/resource/Addr_Translation.png matches
lab/lab2.md:Create a file `lab2.txt` in the top-level xk directory with
lab/lab1.md:Create a file `lab1.txt` in the top-level xk directory with

// -*- tab-width:8; indent-tabs-mode:nil; c-basic-offset:4; -*-
// vim: set sw=4 ts=8 sts=4 et:
/*
 * Copyright (c)
 *   2017 FAU -- Simone Müller <simone.mueller@fau.de>
 *   2017 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2020 FAU -- Martin Letras <martin.letras@fau.de>
 *   2020 FAU -- Joachim Falk <joachim.falk@fau.de>
 * 
 *   This library is free software; you can redistribute it and/or modify it under
 *   the terms of the GNU Lesser General Public License as published by the Free
 *   Software Foundation; either version 2 of the License, or (at your option) any
 *   later version.
 * 
 *   This library is distributed in the hope that it will be useful, but WITHOUT
 *   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 *   FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 *   details.
 * 
 *   You should have received a copy of the GNU Lesser General Public License
 *   along with this library; if not, write to the Free Software Foundation, Inc.,
 *   59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
 */

package de.fau.scd.VPC.helper;

import java.io.IOException;
import java.io.File;
import java.nio.file.Files;

import net.sf.opendse.model.Specification;

public class TempDirectoryHandler {
    private static final String TEMP_DIRECTORY = "TEMP_DIRECTORY_VPC";

    private static TempDirectoryHandler topTempDirectory = null;

    private Integer tempDirectoryCounter = 0;

    synchronized private int getTempDirectoryCounter() {
        return tempDirectoryCounter++;
    }

    private File tempDirectory;

    private TempDirectoryCleanups tempDirectoryCleanups;

    private TempDirectoryHandler(TempDirectoryHandler parent, Specification implementation) throws IOException {
        tempDirectory = new File(parent.getDirectory(), String.format("dse_%05d", parent.getTempDirectoryCounter()));
        tempDirectory.mkdir();
        tempDirectory.deleteOnExit();
        implementation.setAttribute(TEMP_DIRECTORY, this);
    }

    public TempDirectoryHandler(File file) {
        tempDirectory = file;
        tempDirectory.mkdir();
        tempDirectory.deleteOnExit();
    }

    public File getDirectory() {
        return tempDirectory;
    }

    /*
     * (non-Javadoc)
     *
     * @see de.cs12.dse.model.IImplementation#getTempDirectory()
     */
    public static TempDirectoryHandler getTempDirectoryHandler(Specification implementation) {
        TempDirectoryHandler tempDirectoryHandler = implementation.getAttribute(TEMP_DIRECTORY);
        if (tempDirectoryHandler != null)
            return tempDirectoryHandler;
        else {
            try {
                return new TempDirectoryHandler(getTempDirectoryHandler(), implementation);
            } catch (IOException e1) {
                throw new RuntimeException("Could not create temporary diectory!");
            }
        }
    }

    synchronized public static TempDirectoryHandler getTempDirectoryHandler() {
        if (topTempDirectory == null) {
            try {
                topTempDirectory = new TempDirectoryHandler(Files.createTempDirectory("opendse-vpc-").toFile());
            } catch (IOException e1) {
                throw new RuntimeException("Could not create temporary diectory!");
            }
        }
        return topTempDirectory;
    }

    /**
     * Add a customized clean up for the allocated temporary directory.
     *
     * @param tempDirectoryCleanup
     *            customized cleanup
     */
    public boolean addCleanup(ITempDirectoryCleanup tempDirectoryCleanup) {
        if (tempDirectory == null) {
            return false;
        } else {
            if (tempDirectoryCleanups == null) {
                tempDirectoryCleanups = new TempDirectoryCleanups(tempDirectory);
            }
            return tempDirectoryCleanups.add(tempDirectoryCleanup);
        }
    }

    /*
     * (non-Javadoc)
     *
     * @see java.lang.Object#finalize()
     */
    @Override
    protected void finalize() throws Throwable {
        // System.err.println("TempDirectoryHandler.finalize() for
        // "+getDirectory());
        if (tempDirectoryCleanups == null) {
            delete(tempDirectory);
        } else {
            tempDirectoryCleanups.cleanUpTmpDir();
        }
        super.finalize();
    }

    /**
     * Recursive deletion of the temporary directory.
     *
     * @param f
     *            the directory
     */
    private void delete(File f) {
        if (f.isDirectory()) {
            for (File c : f.listFiles())
                delete(c);
        }
        f.delete();
    }
}
